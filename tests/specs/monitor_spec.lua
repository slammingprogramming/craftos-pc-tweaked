-- SPDX-FileCopyrightText: 2026 slammingprogramming
--
-- SPDX-License-Identifier: AGPL-3.0-or-later

-- Monitors, following MonitorPeripheral and TermMethods (Java).
-- Monitors need a window (or a terminal): in a headless run the emulator refuses to create one, and these tests are pending.
local windowed = pcall(function() assert(periphemu.create("left", "monitor")) periphemu.remove("left") end)
local it = windowed and it or pending

describe("A monitor", function()
    local monitor
    before_each(function()
        if windowed then
            periphemu.create("left", "monitor")
            monitor = peripheral.wrap("left")
        end
    end)

    it("validates the text scale", function()
        expect.error(monitor.setTextScale):eq("bad argument #1 (number expected, got nil)")
        expect.error(monitor.setTextScale, 0):eq("Expected number in range 0.5-5")
        expect.error(monitor.setTextScale, 0.4):eq("Expected number in range 0.5-5")
        expect.error(monitor.setTextScale, 5.5):eq("Expected number in range 0.5-5")
        periphemu.remove("left")
    end)

    it("cuts the text scale off to halves", function()
        monitor.setTextScale(1)
        expect(monitor.getTextScale()):eq(1)
        monitor.setTextScale(2.4)
        expect(monitor.getTextScale()):eq(2)
        monitor.setTextScale(2.5)
        expect(monitor.getTextScale()):eq(2.5)
        monitor.setTextScale(5)
        expect(monitor.getTextScale()):eq(5)
        periphemu.remove("left")
    end)

    it("is a colour terminal that behaves like one", function()
        expect(monitor.isColour()):eq(true)
        expect(monitor.isColor()):eq(true)
        local w, h = monitor.getSize()
        expect(w > 0 and h > 0):eq(true)
        monitor.clear()
        monitor.setCursorPos(1, 1)
        monitor.write("ab")
        expect({ monitor.getCursorPos() }):same { 3, 1 }
        monitor.setTextColour(colours.red)
        expect(monitor.getTextColour()):eq(colours.red)
        monitor.setBackgroundColour(colours.blue)
        expect(monitor.getBackgroundColour()):eq(colours.blue)
        monitor.setCursorBlink(true)
        expect(monitor.getCursorBlink()):eq(true)
        periphemu.remove("left")
    end)

    it("writes any value as text, numbers the way Java prints them", function()
        monitor.clear()
        monitor.setCursorPos(1, 1)
        local function advance(value)
            local before = monitor.getCursorPos()
            monitor.write(value)
            local after = monitor.getCursorPos()
            return after - before
        end
        expect(advance(nil)):eq(#"nil")
        expect(advance(true)):eq(#"true")
        expect(advance(1.5)):eq(#"1.5")
        expect(advance(10)):eq(#"10")
        expect(advance(2 ^ 40)):eq(#"1.099511627776E12")
        expect(advance(-0.00001)):eq(#"-1.0E-5")
        expect(advance(1234567.5)):eq(#"1234567.5")
        expect(advance(12345678.5)):eq(#"1.23456785E7")
        periphemu.remove("left")
    end)

    it("moves the cursor by the length of the text even past the edge", function()
        local w = monitor.getSize()
        monitor.setCursorPos(w - 1, 1)
        monitor.write("abcdef")
        expect(monitor.getCursorPos()):eq(w - 1 + 6)
        periphemu.remove("left")
    end)

    it("validates its arguments like the terminal does", function()
        expect.error(monitor.setCursorPos, "a", 1):eq("bad argument #1 (number expected, got string)")
        expect.error(monitor.setTextColour, 0):eq("Colour out of range")
        expect.error(monitor.blit, "ab", "0", "00"):eq("Arguments must be the same length")
        periphemu.remove("left")
    end)
end)
