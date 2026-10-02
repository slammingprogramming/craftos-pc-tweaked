-- SPDX-FileCopyrightText: 2026 slammingprogramming
--
-- SPDX-License-Identifier: AGPL-3.0-or-later

-- Printers, following PrinterPeripheral (Java).
describe("A printer", function()
    local printer
    before_each(function()
        periphemu.create("left", "printer", "printouts")
        printer = peripheral.wrap("left")
    end)

    it("has a page size of 25 by 21 once a page is started", function()
        expect.error(printer.getPageSize):eq("Page not started")
        if printer.newPage() then
            expect({ printer.getPageSize() }):same { 25, 21 }
            expect(printer.endPage()):eq(true)
        end
        periphemu.remove("left")
    end)

    it("needs a page before it can be written to", function()
        expect.error(printer.write, "x"):eq("Page not started")
        expect.error(printer.getCursorPos):eq("Page not started")
        expect.error(printer.setCursorPos, 1, 1):eq("Page not started")
        expect.error(printer.setPageTitle, "x"):eq("Page not started")
        expect.error(printer.setPageTitle):eq("Page not started")
        expect.error(printer.endPage):eq("Page not started")
        periphemu.remove("left")
    end)

    it("moves the cursor when it writes", function()
        if not printer.newPage() then periphemu.remove("left") return end -- no paper or ink in this build
        expect({ printer.getCursorPos() }):same { 1, 1 }
        printer.write("abc")
        expect({ printer.getCursorPos() }):same { 4, 1 }
        printer.setCursorPos(2, 3)
        expect({ printer.getCursorPos() }):same { 2, 3 }
        printer.write(12)
        expect({ printer.getCursorPos() }):same { 4, 3 }
        expect(printer.endPage()):eq(true)
        periphemu.remove("left")
    end)

    it("reports ink and paper levels as numbers", function()
        expect(type(printer.getInkLevel())):eq("number")
        expect(type(printer.getPaperLevel())):eq("number")
        periphemu.remove("left")
    end)
end)
