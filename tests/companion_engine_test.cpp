#include "companion_engine.h"
#include "progression.h"

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
}
