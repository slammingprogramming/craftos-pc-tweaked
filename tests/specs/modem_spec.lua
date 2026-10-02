-- SPDX-FileCopyrightText: 2026 slammingprogramming
--
-- SPDX-License-Identifier: AGPL-3.0-or-later

-- Modems, following ModemPeripheral and ModemState (Java).
describe("A modem", function()
    local modem
    before_each(function()
        periphemu.create("left", "modem")
        modem = peripheral.wrap("left")
    end)

    local function cleanup() modem.closeAll() periphemu.remove("left") periphemu.remove("right") end

    it("validates channels", function()
        expect.error(modem.open, -1):eq("Expected number in range 0-65535")
        expect.error(modem.open, 65536):eq("Expected number in range 0-65535")
        expect.error(modem.isOpen, 65536):eq("Expected number in range 0-65535")
        expect.error(modem.close, -1):eq("Expected number in range 0-65535")
        expect.error(modem.transmit, 65536, 1, "x"):eq("Expected number in range 0-65535")
        expect.error(modem.transmit, 1, -1, "x"):eq("Expected number in range 0-65535")
        expect.error(modem.open):eq("bad argument #1 (number expected, got nil)")
        cleanup()
    end)

    it("opens and closes channels", function()
        expect(modem.isOpen(5)):eq(false)
        modem.open(5)
        expect(modem.isOpen(5)):eq(true)
        modem.open(0)
        modem.open(65535)
        expect(modem.isOpen(65535)):eq(true)
        modem.close(5)
        expect(modem.isOpen(5)):eq(false)
        modem.closeAll()
        expect(modem.isOpen(0)):eq(false)
        expect(modem.isOpen(65535)):eq(false)
        cleanup()
    end)

    it("can have up to 128 channels open", function()
        for i = 1, 128 do modem.open(i) end
        expect.error(modem.open, 129):eq("Too many open channels")
        modem.open(5) -- an open channel can be opened again
        cleanup()
    end)

    it("is wireless", function()
        expect(modem.isWireless()):eq(true)
        cleanup()
    end)

    it("delivers a message to the other modem, but not to itself", function()
        periphemu.create("right", "modem")
        local other = peripheral.wrap("right")
        modem.open(1)
        other.open(2)
        modem.transmit(2, 1, { "hello", 42 })
        local timer = os.startTimer(1)
        local got
        while true do
            local event = table.pack(os.pullEvent())
            if event[1] == "modem_message" then got = event break
            elseif event[1] == "timer" and event[2] == timer then break end
        end
        expect(got ~= nil):eq(true)
        expect(got[2]):eq("right")
        expect(got[3]):eq(2)
        expect(got[4]):eq(1)
        expect(got[5]):same { "hello", 42 }
        expect(type(got[6])):eq("number")
        -- the sender had channel 1 open, but a modem never hears its own message
        modem.transmit(1, 1, "echo")
        local timer2 = os.startTimer(0.5)
        local heard = false
        while true do
            local event = table.pack(os.pullEvent())
            if event[1] == "modem_message" then heard = true break
            elseif event[1] == "timer" and event[2] == timer2 then break end
        end
        expect(heard):eq(false)
        cleanup()
    end)
end)
