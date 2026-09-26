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
}
