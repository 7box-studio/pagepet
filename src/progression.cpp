#include "progression.h"

#include <cmath>

namespace companion {

uint16_t xpForNextLevel(uint8_t level) {
  if (level >= 99) return 0;
  if (level == 0) level = 1;
  return static_cast<uint16_t>(
      std::ceil(160.0 + 120.0 * std::log2(static_cast<double>(level) + 1.0)));
}

uint8_t levelForXp(uint32_t total_xp) {
  uint8_t level = 1;
  while (level < 99) {
    const uint16_t threshold = xpForNextLevel(level);
    if (total_xp < threshold) break;
    total_xp -= threshold;
    ++level;
  }
  return level;
}

Mood moodAt(const CompanionState& state, uint32_t unix_seconds,
            bool clock_trusted) {
  if (!clock_trusted || state.last_read_unix == 0 ||
      unix_seconds < state.last_read_unix)
    return Mood::Unknown;
  const uint32_t idle_hours = (unix_seconds - state.last_read_unix) / 3600u;
  if (idle_hours >= 72) return Mood::Hibernating;
  if (idle_hours >= 48) return Mood::Sleepy;
  if (idle_hours >= 24) return Mood::Quiet;
  return Mood::Happy;
}

}
