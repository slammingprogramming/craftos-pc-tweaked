-- SPDX-FileCopyrightText: 2026 slammingprogramming
--
-- SPDX-License-Identifier: AGPL-3.0-or-later

-- Checks the redstone API against what CC: Tweaked's RedstoneAPI and RedstoneMethods (Java) do.
local sides = { "bottom", "top", "back", "front", "right", "left" }

describe("The redstone library", function()
    local function reset()
        for _, side in ipairs(sides) do
            rs.setAnalogOutput(side, 0)
            rs.setBundledOutput(side, 0)
        end
    end

    it("is available as rs and redstone", function()
        expect(rs):eq(redstone)
        expect(type(redstone.setAnalogueOutput)):eq("function")
        expect(type(redstone.getAnalogueOutput)):eq("function")
        expect(type(redstone.getAnalogueInput)):eq("function")
    end)

    describe("redstone.getSides", function()
        it("returns the six sides in CC: Tweaked's order", function()
            expect(redstone.getSides()):same(sides)
        end)
    end)

    describe("argument validation", function()
        for _, name in ipairs { "getInput", "getOutput", "getAnalogInput", "getAnalogOutput", "getBundledInput", "getBundledOutput" } do
            it(name .. " validates its side", function()
                expect.error(redstone[name]):eq("bad argument #1 (string expected, got nil)")
                expect.error(redstone[name], "sideways"):eq("bad argument #1 (unknown option sideways)")
            end)
        end

        it("rejects a bad analog strength", function()
            expect.error(redstone.setAnalogOutput, "top"):eq("bad argument #2 (number expected, got nil)")
            expect.error(redstone.setAnalogOutput, "top", 16):eq("Expected number in range 0-15")
            expect.error(redstone.setAnalogOutput, "top", -1):eq("Expected number in range 0-15")
        end)

        it("rejects a bad bundled output", function()
            expect.error(redstone.setBundledOutput, "top"):eq("bad argument #2 (number expected, got nil)")
            expect.error(redstone.testBundledInput, "top"):eq("bad argument #2 (number expected, got nil)")
        end)
    end)

    describe("digital and analog output", function()
        it("treats sides case-insensitively", function()
            redstone.setOutput("TOP", true)
            expect(redstone.getOutput("top")):eq(true)
            reset()
        end)

        it("setOutput switches between 0 and 15", function()
            redstone.setOutput("front", true)
            expect(redstone.getAnalogOutput("front")):eq(15)
            expect(redstone.getOutput("front")):eq(true)
            redstone.setOutput("front", false)
            expect(redstone.getAnalogOutput("front")):eq(0)
            expect(redstone.getOutput("front")):eq(false)
        end)

        it("setAnalogOutput sets the strength", function()
            redstone.setAnalogOutput("left", 7)
            expect(redstone.getAnalogOutput("left")):eq(7)
            expect(redstone.getOutput("left")):eq(true)
            redstone.setAnalogOutput("left", 0)
            expect(redstone.getOutput("left")):eq(false)
            reset()
        end)

        it("setAnalogOutput truncates fractions", function()
            redstone.setAnalogOutput("left", 7.9)
            expect(redstone.getAnalogOutput("left")):eq(7)
            reset()
        end)

        it("keeps sides independent", function()
            redstone.setAnalogOutput("left", 3)
            expect(redstone.getAnalogOutput("right")):eq(0)
            reset()
        end)

        it("has no input by default", function()
            for _, side in ipairs(sides) do
                expect(redstone.getInput(side)):eq(false)
                expect(redstone.getAnalogInput(side)):eq(0)
                expect(redstone.getBundledInput(side)):eq(0)
            end
        end)
    end)

    describe("bundled cables", function()
        it("stores the bundled output", function()
            redstone.setBundledOutput("back", 0x0005)
            expect(redstone.getBundledOutput("back")):eq(5)
            redstone.setBundledOutput("back", 0)
            expect(redstone.getBundledOutput("back")):eq(0)
        end)

        it("testBundledInput checks a mask against the input", function()
            -- with no input, only the empty mask is a subset
            expect(redstone.testBundledInput("back", 0)):eq(true)
            expect(redstone.testBundledInput("back", 1)):eq(false)
        end)
    end)
end)
