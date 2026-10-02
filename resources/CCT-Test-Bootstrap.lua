-- SPDX-FileCopyrightText: 2019 JackMacWindows
-- SPDX-FileCopyrightText: 2026 slammingprogramming
--
-- SPDX-License-Identifier: AGPL-3.0-or-later

-- Runs CC: Tweaked's own test suite (McFly, from projects/core/src/test/resources/test-rom) inside the emulator.
--
--   craftos-tweaked --headless --mount-ro test-rom=<path to test-rom> --script resources/CCT-Test-Bootstrap.lua [spec path | debugger]
--
-- The optional argument is a spec file or folder inside the mounted test-rom (default: everything, /test-rom/spec).
-- The emulator exits with the number of failing tests, or 255 if the run did not finish.
local arg = ...
config.set("computerSpaceLimit", 10000000) -- CC: Tweaked's fs tests expect a finite, but large, limit
config.add("http_blacklist", "$private")
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
            if path == target or (target:sub(1, #path + 1) == path .. "/") or not fs.combine(dir):find("^test%-rom/spec") then
                result[#result + 1] = name
            end
        end
        return result
    end
    arg = "/test-rom/spec"
end
shell.run("/test-rom/mcfly " .. (arg or "/test-rom/spec"))
logfile:close()

-- McFly prints "Ran N test(s), of which M passed (P%)." when it finishes
local log = assert(io.open("test-log.txt", "r"))
local text = log:read("*a")
log:close()
local ran, passed = text:match("Ran (%d+) test%(s%), of which (%d+) passed")
if ran then os.shutdown(tonumber(ran) - tonumber(passed)) else os.shutdown(255) end
