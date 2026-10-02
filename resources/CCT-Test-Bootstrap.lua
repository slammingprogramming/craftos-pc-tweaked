-- SPDX-FileCopyrightText: 2019 JackMacWindows
-- SPDX-FileCopyrightText: 2026 slammingprogramming
--
-- SPDX-License-Identifier: AGPL-3.0-or-later

-- Runs CC: Tweaked's own test suite (McFly, from projects/core/src/test/resources/test-rom) inside the emulator.
--
--   craftos-tweaked --headless --mount-ro test-rom=<path to test-rom> --script resources/CCT-Test-Bootstrap.lua --args "[spec path | debugger] [spec folder]"
--
-- The optional argument is a spec file or folder inside the mounted test-rom (default: everything, /test-rom/spec). The
-- second argument is the folder McFly runs when it is not /test-rom/spec: CraftOS-Tweaked's own specs (tests/specs) are
-- mounted as /emu-spec and run with "/emu-spec/redstone_spec.lua /emu-spec".
-- The emulator exits with the number of failing tests, or 255 if the run did not finish.
local arg, root = ...
root = fs.combine(root or "/test-rom/spec") -- the folder McFly runs (CraftOS-Tweaked's own specs live in another one)
config.set("computerSpaceLimit", 10000000) -- CC: Tweaked's fs tests expect a finite, but large, limit
-- CC: Tweaked's test computer has a modem on top (its peripheral specs check it)
periphemu.create("top", "modem")
-- ... and a peripheral hub below it with one remote peripheral, "remote_1" (ComputerTestDelegate.FakePeripheralHub)
periphemu.create("bottom", "scripted", {
    type = "peripheral_hub",
    methods = {
        getNamesRemote = function() return { "remote_1" } end,
        isPresentRemote = function(name) return name == "remote_1" end,
        getTypeRemote = function(name) if name == "remote_1" then return "remote", "other_type" end end,
        hasTypeRemote = function(name, type) if name == "remote_1" then return type == "remote" or type == "other_type" end end,
        getMethodsRemote = function(name) if name == "remote_1" then return { "func" } end end,
        callRemote = function(name, method) end,
    },
})
if arg == "debugger" then
    periphemu.create("left", "debugger")
    peripheral.call("left", "break")
    arg = nil
end
for _,v in ipairs(fs.list("/")) do if not fs.isReadOnly(v) then fs.delete(v) end end
-- CC: Tweaked's own test computer has a /startup.lua, which some fs specs use as an example file
local startup = assert(fs.open("startup.lua", "w"))
startup.write("-- test computer startup file\n")
startup.close()
_G._CCPC_FIRST_RUN = nil
_G._CCPC_UPDATED_VERSION = nil
local logfile = assert(io.open("test-log.txt", "w"))
io.output(logfile)
if arg and arg:sub(-9) == "_spec.lua" then
    -- McFly can only run a folder: hide every other spec file from it so that just this one runs
    local target = fs.combine(arg)
    local list = fs.list
    function fs.list(dir)
        local ok, names = pcall(list, dir)
        if not ok then error(names, 0) end
        local result = {}
        for _, name in ipairs(names) do
            local path = fs.combine(dir, name)
            local inside = fs.combine(dir) == root or fs.combine(dir):sub(1, #root + 1) == root .. "/"
            if path == target or (target:sub(1, #path + 1) == path .. "/") or not inside then
                result[#result + 1] = name
            end
        end
        return result
    end
    arg = "/" .. root
end
shell.run("/test-rom/mcfly " .. (arg or ("/" .. root)))
logfile:close()

-- McFly prints "Ran N test(s), of which M passed (P%)." when it finishes
local log = assert(io.open("test-log.txt", "r"))
local text = log:read("*a")
log:close()
local ran, passed = text:match("Ran (%d+) test%(s%), of which (%d+) passed")
config.set("standardsMode", false) -- in this mode a window would stay open after the shutdown
if ran then os.shutdown(tonumber(ran) - tonumber(passed)) else os.shutdown(255) end
