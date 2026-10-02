-- SPDX-FileCopyrightText: 2026 slammingprogramming
--
-- SPDX-License-Identifier: AGPL-3.0-or-later

-- Speakers, following SpeakerPeripheral (Java). Nothing here needs sound output.
describe("A speaker", function()
    local speaker
    before_each(function()
        periphemu.create("left", "speaker")
        speaker = peripheral.wrap("left")
    end)

    it("plays notes", function()
        expect(speaker.playNote("harp")):eq(true)
        expect(speaker.playNote("HARP", 2, 12)):eq(true)
        expect(speaker.playNote("pling", 3, 24)):eq(true)
        periphemu.remove("left")
    end)

    it("validates notes", function()
        expect.error(speaker.playNote):eq("bad argument #1 (string expected, got nil)")
        expect.error(speaker.playNote, "harp", "loud"):eq("bad argument #2 (number expected, got string)")
        expect.error(speaker.playNote, "kazoo"):eq("Invalid instrument, \"null\"!") -- CC: Tweaked prints the instrument it did not find
        expect.error(speaker.playNote, "harp", 1, 0 / 0):eq("bad argument #3 (number expected, got nan)")
        periphemu.remove("left")
    end)

    it("validates sounds", function()
        expect.error(speaker.playSound):eq("bad argument #1 (string expected, got nil)")
        expect.error(speaker.playSound, ("a"):rep(513)):eq("bad argument #1 (sound name is too long)")
        expect.error(speaker.playSound, "Not A Sound"):eq("bad argument #1 (malformed sound name)")
        periphemu.remove("left")
    end)

    it("validates audio", function()
        expect.error(speaker.playAudio):eq("bad argument #1 (table expected, got nil)")
        expect.error(speaker.playAudio, {}):eq("Cannot play empty audio")
        local big = {}
        for i = 1, 128 * 1024 + 1 do big[i] = 0 end
        expect.error(speaker.playAudio, big):eq("Audio data is too large")
        periphemu.remove("left")
    end)

    it("can be stopped", function()
        speaker.stop()
        periphemu.remove("left")
    end)
end)
