-- SPDX-FileCopyrightText: 2026 slammingprogramming
--
-- SPDX-License-Identifier: AGPL-3.0-or-later

-- The computer peripheral (another computer attached to this one), following ComputerPeripheral (Java).
describe("A computer peripheral", function()
    it("is named after the computer's ID and reports its state", function()
        expect(periphemu.create(77, "computer")):eq(true)
        expect(peripheral.getType("computer_77")):eq("computer")
        local other = peripheral.wrap("computer_77")
        expect(other.getID()):eq(77)
        expect(other.getLabel()):eq(nil)
        expect(other.isOn()):eq(true) -- the emulator starts the computer when it is attached
        other.shutdown()
        local deadline = os.clock() + 5
        while other.isOn() and os.clock() < deadline do os.sleep(0.05) end
        expect(not other.isOn()):eq(true) -- (a computer that is off may also be gone from the peripheral)
        periphemu.remove("computer_77")
    end)
end)
