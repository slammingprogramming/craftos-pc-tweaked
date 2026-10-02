-- SPDX-FileCopyrightText: 2026 slammingprogramming
--
-- SPDX-License-Identifier: AGPL-3.0-or-later

-- Peripherals that the emulator can create (periphemu) and what the peripheral API does with them, following CC: Tweaked's
-- PeripheralAPI (Java) and rom/apis/peripheral.lua.
describe("The peripheral API with emulated peripherals", function()
    local function with(side, kind, fn)
        expect(periphemu.create(side, kind)):eq(true)
        local ok, err = pcall(fn)
        periphemu.remove(side)
        if not ok then error(err, 0) end
    end

    it("lists, wraps and removes a peripheral", function()
        with("left", "speaker", function()
            expect(peripheral.isPresent("left")):eq(true)
            expect(peripheral.getType("left")):eq("speaker")
            expect(peripheral.hasType("left", "speaker")):eq(true)
            expect(peripheral.hasType("left", "monitor")):eq(false)
            local found = false
            for _, name in ipairs(peripheral.getNames()) do if name == "left" then found = true end end
            expect(found):eq(true)
            expect(peripheral.getName(peripheral.wrap("left"))):eq("left")
        end)
        expect(peripheral.isPresent("left")):eq(false)
        expect(peripheral.wrap("left")):eq(nil)
    end)

    -- events of earlier tests may still be queued: wait for the one about this side
    local function pull(event, side)
        local timer = os.startTimer(2)
        while true do
            local e, a = os.pullEvent()
            if e == event and a == side then return a end
            if e == "timer" and a == timer then return nil end
        end
    end

    it("queues peripheral and peripheral_detach events", function()
        expect(periphemu.create("right", "speaker")):eq(true)
        expect(pull("peripheral", "right")):eq("right")
        periphemu.remove("right")
        expect(pull("peripheral_detach", "right")):eq("right")
    end)

    it("lists the methods of a peripheral", function()
        with("left", "speaker", function()
            local methods = {}
            for _, name in ipairs(peripheral.getMethods("left")) do methods[name] = true end
            for _, name in ipairs { "playAudio", "playNote", "playSound", "stop" } do expect(methods[name]):eq(true) end
        end)
    end)

    it("reports the same methods on a wrapped peripheral", function()
        with("left", "speaker", function()
            local wrapped = peripheral.wrap("left")
            for _, name in ipairs { "playAudio", "playNote", "playSound", "stop" } do
                expect(type(wrapped[name])):eq("function")
            end
        end)
    end)

    it("fails on a method that does not exist", function()
        with("left", "speaker", function()
            expect.error(peripheral.call, "left", "nonexistent"):eq("No such method nonexistent")
        end)
    end)

    it("does nothing for a missing or removed peripheral", function()
        expect(peripheral.call("left", "playNote")):eq(nil) -- rom/apis/peripheral.lua checks that it is there first
        expect(periphemu.create("left", "speaker")):eq(true)
        local speaker = peripheral.wrap("left")
        periphemu.remove("left")
        expect(speaker.stop()):eq(nil)
    end)

    it("reports errors where peripheral.call was invoked", function()
        with("left", "speaker", function()
            expect.error(function() peripheral.call("left", "playNote", false) end)
                :str_match("^[^:]+:%d+: bad argument #1 %(string expected, got boolean%)$")
        end)
    end)

    it("does not attach to a side that already has a peripheral", function()
        with("left", "speaker", function()
            expect(periphemu.create("left", "speaker")):eq(false)
        end)
    end)

    it("rejects an unknown peripheral type", function()
        expect(periphemu.create("left", "not_a_peripheral")):eq(false)
        expect(peripheral.isPresent("left")):eq(false)
    end)
end)
