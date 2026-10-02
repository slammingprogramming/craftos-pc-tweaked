-- SPDX-FileCopyrightText: 2026 slammingprogramming
--
-- SPDX-License-Identifier: AGPL-3.0-or-later

-- Every method that CC: Tweaked's peripherals have must exist on the emulated ones (they may have more: CraftOS-PC adds
-- some). The lists come from the @LuaFunction methods of the Java classes, as of CC: Tweaked 1.120.
local required = {
    monitor = { "write", "scroll", "setCursorPos", "setCursorBlink", "getCursorPos", "getCursorBlink", "getSize", "clear",
        "clearLine", "setTextColour", "setTextColor", "setBackgroundColour", "setBackgroundColor", "isColour", "isColor",
        "getTextColour", "getTextColor", "getBackgroundColour", "getBackgroundColor", "blit", "setPaletteColour",
        "setPaletteColor", "getPaletteColour", "getPaletteColor", "setTextScale", "getTextScale" },
    drive = { "isDiskPresent", "getDiskLabel", "setDiskLabel", "hasData", "getMountPath", "hasAudio", "getAudioTitle",
        "playAudio", "stopAudio", "ejectDisk", "getDiskID" },
    printer = { "write", "getCursorPos", "setCursorPos", "getPageSize", "newPage", "endPage", "setPageTitle", "getInkLevel",
        "getPaperLevel" },
    modem = { "open", "isOpen", "close", "closeAll", "transmit", "isWireless" },
    computer = { "turnOn", "shutdown", "reboot", "getID", "isOn", "getLabel" },
    speaker = { "playNote", "playSound", "playAudio", "stop" },
}

describe("The emulated peripherals", function()
    for kind, names in pairs(required) do
        it("have all of CC: Tweaked's methods: " .. kind, function()
            local side = kind == "computer" and 42 or "left" -- a computer peripheral is named after the computer's ID
            local name = kind == "computer" and "computer_42" or "left"
            local ok, err = pcall(periphemu.create, side, kind, kind == "printer" and "printouts" or nil)
            if not ok and tostring(err):find("not available in this mode") then return end -- no window in this run
            expect(ok and err):eq(true)
            local methods = {}
            for _, name in ipairs(peripheral.getMethods(name)) do methods[name] = true end
            local missing = {}
            for _, name in ipairs(names) do if not methods[name] then missing[#missing + 1] = name end end
            local wrapped = peripheral.getType(name)
            periphemu.remove(side)
            expect(wrapped):eq(kind)
            expect(missing):same {}
        end)
    end
end)
