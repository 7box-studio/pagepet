#pragma once

#include "companion_state.h"

#include <cstddef>
#include <cstdint>

namespace companion {

constexpr size_t kStateRecordSize = 38;

/** @brief Encode a versioned little-endian state record with a checksum. */
bool encodeState(const CompanionState& state, uint8_t* out, size_t length);
/** @brief Decode a valid record; leave @p out unchanged on failure. */
bool decodeState(const uint8_t* data, size_t length, CompanionState& out);

}
