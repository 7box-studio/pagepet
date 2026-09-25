#pragma once

#include "companion_state.h"

namespace companion {

/** @brief XP required to advance from a level, or zero at the level cap. */
uint16_t xpForNextLevel(uint8_t level);
/** @brief Derive a level in the inclusive range 1-99 from lifetime XP. */
uint8_t levelForXp(uint32_t total_xp);
/** @brief Derive mood without mutating or persisting state. */
Mood moodAt(const CompanionState& state, uint32_t unix_seconds,
            bool clock_trusted);

}
