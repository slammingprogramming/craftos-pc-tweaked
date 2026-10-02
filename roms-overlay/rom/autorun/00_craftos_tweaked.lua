-- SPDX-FileCopyrightText: 2019 JackMacWindows
-- SPDX-FileCopyrightText: 2026 slammingprogramming
--
-- SPDX-License-Identifier: AGPL-3.0-or-later

--[[
CraftOS-Tweaked hooks.

CC: Tweaked's startup.lua runs every file in /rom/autorun before the user's
startup files. This file is how the emulator hooks into the unmodified CC:
Tweaked ROM: it restores os.shutdown's exit code, runs the --script/--exec
code from the command line, adds shell completion for the emulator's own
programs, and reports plugin errors.
]]

local completion = require "cc.shell.completion"

-- os.shutdown(code): bios.lua's wrapper drops the exit code (see prelude.lua)
local nativeShutdown = _CCPC_nativeShutdown
_G._CCPC_nativeShutdown = nil
if nativeShutdown then
    function os.shutdown(...)
        nativeShutdown(...)
        while true do coroutine.yield() end
    end
end

settings.define("shell.report_plugin_errors", {
    default = true,
    description = "Show errors on startup if a plugin(s) failed to load.",
    type = "boolean",
})
settings.define("shell.mobile_resize_with_keyboard", {
    default = true,
    description = "Automatically resize the shell when the keyboard is opened or closed.",
    type = "boolean",
})

-- Programs that only exist in the emulator
local function completeConfigPart2(shell, text, previous)
    if previous[2] == "get" or previous[2] == "set" then
        return completion.choice(shell, text, previous, config.list(), previous[2] == "set")
    end
end

local function completeConfigPart3(shell, text, previous)
    if previous[2] == "set" then
        if config.getType(previous[3]) == "boolean" then return completion.choice(shell, text, previous, {"true", "false"})
        elseif previous[3] == "mount_mode" then return completion.choice(shell, text, previous, {"none", "ro", "ro_strict", "rw"}) end
    end
end

local function completeGistPut(shell, text, previous)
    if previous[2] == "put" then
        return fs.complete(text, shell.dir(), true, false)
    end
end

shell.setCompletionFunction("rom/programs/http/gist.lua", completion.build(
    { completion.choice, { "put ", "get ", "run ", "edit ", "info ", "delete " } },
    completeGistPut
))

if periphemu and config and mounter then
    shell.setCompletionFunction("rom/programs/attach.lua", completion.build(
        completion.peripheral,
        { completion.choice, periphemu.names() }
    ))
    shell.setCompletionFunction("rom/programs/detach.lua", completion.build(completion.peripheral))
    shell.setCompletionFunction("rom/programs/config.lua", completion.build(
        { completion.choice, { "get ", "set ", "list " } },
        completeConfigPart2,
        completeConfigPart3
    ))
    shell.setCompletionFunction("rom/programs/mount.lua", completion.build(completion.dir))
    shell.setCompletionFunction("rom/programs/unmount.lua", completion.build(completion.dir))
end

-- Run the code passed with --script/--exec, if there is any
if _CCPC_STARTUP_SCRIPT then
    local fn, err = load(_CCPC_STARTUP_SCRIPT, "@startup.lua", "t", _ENV)
    if fn then
        local args = {}
        if _CCPC_STARTUP_ARGS then for n in _CCPC_STARTUP_ARGS:gmatch("[^ ]+") do table.insert(args, n) end end
        local oldpath
        if shell then
            local dir = shell.dir()
            if dir:sub(1, 1) ~= "/" then dir = "/" .. dir end
            if dir:sub(-1) ~= "/" then dir = dir .. "/" end

            local strip_path = "?;?.lua;?/init.lua;"
            local path = package.path
            if path:sub(1, #strip_path) == strip_path then
                path = path:sub(#strip_path + 1)
            end

            oldpath = package.path
            package.path = dir .. "?;" .. dir .. "?.lua;" .. dir .. "?/init.lua;" .. path
        end
        fn(table.unpack(args))
        if oldpath then package.path = oldpath end
    else printError("Could not load startup script: " .. err) end
end

-- Mobile builds show an introduction the first time they run
if mobile and _CCPC_FIRST_RUN then
    shell.run("/rom/programs/mobile/onboarding.lua")
end

if _CCPC_PLUGIN_ERRORS and settings.get("shell.report_plugin_errors") then
    printError("Some plugins failed to load:")
    for k, v in pairs(_CCPC_PLUGIN_ERRORS) do
        printError("  " .. k .. " - " .. v)
    end
end
