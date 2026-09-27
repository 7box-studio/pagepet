#include "companion_engine.h"
#include "progression.h"
#include "character_art.h"

#include <array>
#include <cassert>
#include <cstdint>

using namespace companion;

static PageTurnEvent turn(uint32_t page, uint32_t ms, int32_t day = 20000) {
  return {7, page, ms, 1728000000u + ms / 1000u, day, true, 23, 5};
}

int main() {
  CompanionEngine engine;
  assert(!engine.onPageTurn(turn(1, 0)));
  assert(!engine.onPageTurn(turn(2, 14000)));
  assert(engine.onPageTurn(turn(3, 29000)));
  assert(!engine.onPageTurn(turn(4, 33000)));
  assert(!engine.onPageTurn(turn(10, 53000)));
  assert(!engine.onPageTurn(turn(11, 354000)));
  assert(engine.onPageTurn(turn(12, 369000)));
  assert(engine.state().valid_pages == 2);
  assert(engine.state().achievement_bits & NightReader);
  assert(engine.state().achievement_bits & WeekendReader);
  assert(!engine.flushDue(600000));
  assert(engine.flushDue(629000));

  std::array<uint8_t, CompanionEngine::kRecordSize> bytes{};
  assert(engine.encode(bytes.data(), bytes.size()));
  CompanionEngine restored;
  assert(restored.decode(bytes.data(), bytes.size()));
  assert(restored.state().valid_pages == 2);
  assert(!restored.dirty());
  bytes[8] ^= 1;
  assert(!restored.decode(bytes.data(), bytes.size()));
  assert(restored.state().valid_pages == 2);

  restored.beginSession();
  assert(!restored.onPageTurn(turn(1, 0, 20001)));
  for (uint32_t page = 2; page <= 6; ++page)
    assert(restored.onPageTurn(turn(page, page * 15000, 20001)));
  assert(restored.state().streak_days == 1);
  assert(restored.state().day_pages == 5);
  assert(restored.state().session_pages == 5);
  assert(levelForXp(0) == 1);
  assert(levelForXp(UINT32_MAX) == 99);
  assert(moodAt(restored.state(),
                restored.state().last_read_unix + 72 * 3600, true) ==
         Mood::Hibernating);
  assert(moodAt(restored.state(), 0, false) == Mood::Unknown);
  for (unsigned i = 0; i <= static_cast<unsigned>(CharacterClass::Knight); ++i) {
    const CharacterClass kind = static_cast<CharacterClass>(i);
    std::array<uint8_t, kSpriteBytes> sprite{};
    assert(renderCharacter(kind, sprite.data(), sprite.size()));
    assert(renderCharacter(VisualStyle::Fantasy, kind, sprite.data(), sprite.size()));
    assert(characterName(kind)[0] != '\0');
    bool has_ink = false;
    for (uint8_t byte : sprite) has_ink |= byte != 0;
    assert(has_ink);
    CompanionState selected = restored.state();
    selected.character_class = kind;
    assert(encodeState(selected, bytes.data(), bytes.size()));
    CompanionState decoded;
    assert(decodeState(bytes.data(), bytes.size(), decoded));
    assert(decoded.character_class == kind);
  }
  assert(!renderCharacter(static_cast<CharacterClass>(255), nullptr, 0));
  CompanionState future_style = restored.state();
  future_style.visual_style = VisualStyle::CuteAnimals;
  assert(encodeState(future_style, bytes.data(), bytes.size()));
  CompanionState loaded_style;
  assert(decodeState(bytes.data(), bytes.size(), loaded_style));
  assert(loaded_style.visual_style == VisualStyle::CuteAnimals);
  std::array<uint8_t, kSpriteBytes> unsupported_sprite{};
  assert(!renderCharacter(VisualStyle::CuteAnimals, loaded_style.character_class,
                          unsupported_sprite.data(), unsupported_sprite.size()));

  // A monotonic timer wrap must not invalidate a normal reading interval.
  CompanionEngine wrapped;
  const uint32_t before_wrap = UINT32_MAX - 5000u;
  assert(!wrapped.onPageTurn(turn(20, before_wrap, 20002)));
  assert(wrapped.onPageTurn(turn(21, before_wrap + 15000u, 20002)));

  // UINT32_MAX is a valid page anchor, but it cannot be incremented safely.
  CompanionEngine max_page;
  assert(!max_page.onPageTurn(turn(UINT32_MAX, 0, 20003)));
  assert(!max_page.onPageTurn(turn(0, 15000, 20003)));
  assert(max_page.onPageTurn(turn(1, 30000, 20003)));

  // A failed encode must not write through a too-short destination buffer.
  std::array<uint8_t, CompanionEngine::kRecordSize - 1> short_bytes{};
  assert(!max_page.encode(short_bytes.data(), short_bytes.size()));

  // Reader events are logical pages, not button presses or redraws.
  CompanionEngine navigation;
  navigation.beginSession();
  assert(!navigation.onPageTurn(turn(10, 0)));
  assert(!navigation.onPageTurn(turn(10, 10000)));
  assert(navigation.onPageTurn(turn(11, 15000)));
  PageTurnEvent other_book = turn(12, 45000);
  other_book.book_id = 8;
  assert(!navigation.onPageTurn(other_book));
  other_book.page_index = 13;
  other_book.monotonic_ms = 60000;
  assert(navigation.onPageTurn(other_book));
  assert(!navigation.onPageTurn(turn(100, 75000)));
  assert(!navigation.onPageTurn(turn(99, 90000)));
  assert(navigation.state().valid_pages == 2);

  // The time window is inclusive at both boundaries.
  CompanionEngine timing;
  assert(!timing.onPageTurn(turn(1, 0)));
  assert(timing.onPageTurn(turn(2, 15000)));
  assert(timing.onPageTurn(turn(3, 315000)));
  assert(!timing.onPageTurn(turn(4, 615001)));

  // Untrusted wall time may award page XP but cannot create dated achievements.
  CompanionEngine no_clock;
  PageTurnEvent undated = turn(1, 0);
  undated.clock_trusted = false;
  undated.unix_seconds = 0;
  undated.local_day = -1;
  undated.local_hour = 255;
  undated.weekday = 255;
  assert(!no_clock.onPageTurn(undated));
  undated.page_index = 2;
  undated.monotonic_ms = 15000;
  assert(no_clock.onPageTurn(undated));
  assert(no_clock.state().total_xp == 10);
  assert(no_clock.state().streak_days == 0);
  assert(no_clock.state().achievement_bits == 0);

  // Resetting a saved session changes persistent state and must be committed.
  no_clock.markFlushed();
  no_clock.beginSession(20000);
  assert(no_clock.state().session_pages == 0);
  assert(no_clock.dirty());
  assert(!no_clock.flushDue(20000 + CompanionEngine::kFlushIntervalMs - 1));
  assert(no_clock.flushDue(20000 + CompanionEngine::kFlushIntervalMs));
}
