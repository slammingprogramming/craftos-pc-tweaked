-- SPDX-FileCopyrightText: 2026 slammingprogramming
--
-- SPDX-License-Identifier: AGPL-3.0-or-later

-- APIs that CC: Tweaked only has inside Minecraft because they need its world: a turtle moves and digs, a command
-- computer runs commands, a pocket computer upgrades itself. CraftOS-Tweaked has no world, so they are not provided; this
-- file records that (the tests show as pending) so the gap is measured and tracked instead of forgotten.
describe("Minecraft-only APIs", function()
    local function present(name) return _G[name] ~= nil end
    local it_if_present = function(name, desc, fn) (present(name) and it or pending)(desc, fn) end

    it_if_present("turtle", "turtle API exists on a turtle", function() expect(type(turtle.forward)):eq("function") end)
    it_if_present("commands", "commands API exists on a command computer", function() expect(type(commands.exec)):eq("function") end)
    it_if_present("pocket", "pocket API exists on a pocket computer", function() expect(type(pocket.equipBack)):eq("function") end)
    it("does not provide those APIs on a normal computer", function()
        -- on a normal CC: Tweaked computer these globals do not exist either
        expect(turtle):eq(nil)
        expect(commands):eq(nil)
        expect(pocket):eq(nil)
    end)
end)
