-- SPDX-FileCopyrightText: 2019 JackMacWindows
-- SPDX-FileCopyrightText: 2026 slammingprogramming
--
-- SPDX-License-Identifier: AGPL-3.0-or-later

--[[
CraftOS-Tweaked prelude.

The emulator runs this file once, right before CC: Tweaked's own bios.lua, so
that bios.lua can be used completely unmodified. CC: Tweaked runs on the Cobalt
Lua VM, while CraftOS-Tweaked uses (a modified) PUC Lua 5.2, and this file
fills in the Lua 5.1 features that CC: Tweaked programs expect: load() with an
environment, loadstring, setfenv/getfenv, unpack, bit (a bit32 wrapper),
math.log10, table.maxn and so on. The _CC_DISABLE_LUA51_FEATURES setting turns
the 5.1-only functions off again, like it does in CC: Tweaked.

The shims below come from the bios.lua of CraftOS-PC 2 by JackMacWindows (MIT
License), where they used to live inside a modified copy of CC: Tweaked's bios.
]]

-- bios.lua replaces os.shutdown with a wrapper that drops the exit code the
-- emulator supports (os.shutdown(code)). Keep the original; autorun/00_craftos_tweaked.lua
-- wraps it again after bios.lua has run and then removes this global.
_CCPC_nativeShutdown = os.shutdown

local expect
do
    local h = fs.open("rom/modules/main/cc/expect.lua", "r")
    local f, err = (_VERSION == "Lua 5.1" and loadstring or load)(h.readAll(), "@/rom/modules/main/cc/expect.lua")
    h.close()

    if not f then error(err) end
    expect = f().expect
end

if jit and jit.os == "OSX" and jit.arch == "arm64" then jit.off() end

-- Historically load/loadstring would handle the chunk name as if it has
-- been prefixed with "=". We emulate that behaviour here.
local function prefix(chunkname)
    if type(chunkname) ~= "string" then return chunkname end
    local head = chunkname:sub(1, 1)
    if head == "=" or head == "@" then
        return chunkname
    else
        return "=" .. chunkname
    end
end

if _VERSION == "Lua 5.1" then
    -- If we're on Lua 5.1, install parts of the Lua 5.2/5.3 API so that programs can be written against it
    local type = type
    local nativeload = load
    local nativeloadstring = loadstring
    local nativesetfenv = setfenv

    function load(x, name, mode, env)
        expect(1, x, "function", "string")
        expect(2, name, "string", "nil")
        expect(3, mode, "string", "nil")
        expect(4, env, "table", "nil")

        local ok, p1, p2 = pcall(function()
            if type(x) == "string" then
                local result, err = nativeloadstring(x, name)
                if result then
                    if env then
                        env._ENV = env
                        nativesetfenv(result, env)
                    end
                    return result
                else
                    return nil, err
                end
            else
                local result, err = nativeload(x, name)
                if result then
                    if env then
                        env._ENV = env
                        nativesetfenv(result, env)
                    end
                    return result
                else
                    return nil, err
                end
            end
        end)
        if ok then
            return p1, p2
        else
            error(p1, 2)
        end
    end

    if _CC_DISABLE_LUA51_FEATURES then
        -- Remove the Lua 5.1 features that will be removed when we update to Lua 5.2, for compatibility testing.
        -- See "disable_lua51_functions" in ComputerCraft.cfg
        setfenv = nil
        getfenv = nil
        loadstring = nil
        unpack = nil
        math.log10 = nil
        table.maxn = nil
    else
        loadstring = function(string, chunkname) return nativeloadstring(string, prefix(chunkname)) end

        -- Inject a stub for the old bit library
        _G.bit = {
            bnot = bit32.bnot,
            band = bit32.band,
            bor = bit32.bor,
            bxor = bit32.bxor,
            brshift = bit32.arshift,
            blshift = bit32.lshift,
            blogic_rshift = bit32.rshift,
        }
    end
elseif not _CC_DISABLE_LUA51_FEATURES then
    -- Restore old Lua 5.1 functions for compatibility
    if not getfenv or not setfenv then
        -- setfenv/getfenv replacements from https://leafo.net/guides/setfenv-in-lua52-and-above.html
        function setfenv(fn, env)
            if not debug then error("could not set environment", 2) end
            if type(fn) == "number" then fn = debug.getinfo(fn + 1, "f").func end
            local i = 1
            while true do
                local name = debug.getupvalue(fn, i)
                if name == "_ENV" then
                    debug.upvaluejoin(fn, i, (function()
                        return env
                    end), 1)
                    break
                elseif not name then
                    break
                end

                i = i + 1
            end

            return fn
        end

        function getfenv(fn)
            if not debug then error("could not set environment", 2) end
            if type(fn) == "number" then fn = debug.getinfo(fn + 1, "f").func end
            local i = 1
            while true do
                local name, val = debug.getupvalue(fn, i)
                if name == "_ENV" then
                    return val
                elseif not name then
                    break
                end
                i = i + 1
            end
        end
    end

    function table.maxn(tab)
        local num = 0
        for k in pairs(tab) do
            if type(k) == "number" and k > num then
                num = k
            end
        end
        return num
    end

    math.log10 = function(x) return math.log(x, 10) end
    loadstring = function(string, chunkname) return load(string, prefix(chunkname)) end
    unpack = table.unpack

    -- Inject a stub for the old bit library
    _G.bit = {
        bnot = bit32.bnot,
        band = bit32.band,
        bor = bit32.bor,
        bxor = bit32.bxor,
        brshift = bit32.arshift,
        blshift = bit32.lshift,
        blogic_rshift = bit32.rshift,
    }
end

-- Install lua parts of the os api
function os.version()
    return "CraftOS 1.9"
end

function os.pullEventRaw(sFilter)
    return coroutine.yield(sFilter)
end

function os.pullEvent(sFilter)
    local eventData = table.pack(os.pullEventRaw(sFilter))
    if eventData[1] == "terminate" then
        error("Terminated", 0)
    end
    return table.unpack(eventData, 1, eventData.n)
end

-- Install globals
function sleep(nTime)
    expect(1, nTime, "number", "nil")
    local timer = os.startTimer(nTime or 0)
    repeat
        local _, param = os.pullEvent("timer")
    until param == timer
end

function write(sText)
    expect(1, sText, "string", "number")

    local w, h = term.getSize()
    local x, y = term.getCursorPos()

    local nLinesPrinted = 0
    local function newLine()
        if y + 1 <= h then
            term.setCursorPos(1, y + 1)
        else
            term.setCursorPos(1, h)
            term.scroll(1)
        end
        x, y = term.getCursorPos()
        nLinesPrinted = nLinesPrinted + 1
    end

    -- Print the line with proper word wrapping
    sText = tostring(sText)
    while #sText > 0 do
        local whitespace = string.match(sText, "^[ \t]+")
        if whitespace then
            -- Print whitespace
            term.write(whitespace)
            x, y = term.getCursorPos()
            sText = string.sub(sText, #whitespace + 1)
        end

        local newline = string.match(sText, "^\n")
        if newline then
            -- Print newlines
            newLine()
            sText = string.sub(sText, 2)
        end

        local text = string.match(sText, "^[^ \t\n]+")
        if text then
            sText = string.sub(sText, #text + 1)
            if #text > w then
                -- Print a multiline word
                while #text > 0 do
                    if x > w then
                        newLine()
                    end
                    term.write(text)
                    text = string.sub(text, w - x + 2)
                    x, y = term.getCursorPos()
                end
            else
                -- Print a word normally
                if x + #text - 1 > w then
                    newLine()
                end
                term.write(text)
                x, y = term.getCursorPos()
            end
        end
    end

    return nLinesPrinted
end

function print(...)
    local nLinesPrinted = 0
    local nLimit = select("#", ...)
    for n = 1, nLimit do
        local s = tostring(select(n, ...))
        if n < nLimit then
            s = s .. "\t"
        end
        nLinesPrinted = nLinesPrinted + write(s)
    end
    nLinesPrinted = nLinesPrinted + write("\n")
    return nLinesPrinted
end

function printError(...)
    local oldColour
    if term.isColour() then
        oldColour = term.getTextColour()
        term.setTextColour(colors.red)
    end
    print(...)
    if term.isColour() then
        term.setTextColour(oldColour)
    end
end

