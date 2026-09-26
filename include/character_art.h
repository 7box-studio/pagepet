#pragma once

#include "companion_state.h"

#include <cstddef>
#include <cstdint>

namespace companion {

constexpr uint8_t kSpriteWidth = 16;
constexpr uint8_t kSpriteHeight = 16;
constexpr size_t kSpriteBytes = 32;

/** Draw a 16x16 character into a packed 1-bit bitmap (MSB first, two bytes per row).
 * A set bit is a dark pixel. Returns false for an unknown class or short buffer.
 */
bool renderCharacter(CharacterClass character, uint8_t* bitmap, size_t length);

/** Style-aware entry point for a future settings selector.
 * Cute animal art is not available yet, so that style returns false.
 */
bool renderCharacter(VisualStyle style, CharacterClass character,
                     uint8_t* bitmap, size_t length);

/** English display name for the selected character. */
const char* characterName(CharacterClass character);

}
