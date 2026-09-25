#pragma once

#include <cstdint>

namespace companion {

/** @brief Visual progression branch for the character. */
enum class CharacterClass : uint8_t {
  Wanderer = 0,
  Bibliomancer = 1,
  Mage = 2,
  Archivist = 3,
};

/** @brief Expression derived from time since the last credited page. */
enum class Mood : uint8_t { Happy, Quiet, Sleepy, Hibernating, Unknown };

/** @brief Achievement flags with stable positions in the stored record. */
enum Achievement : uint32_t {
  NightReader = 1u << 0,
  Marathon = 1u << 1,
  WeekendReader = 1u << 2,
};

/** @brief Persistent gameplay state; serialized by encodeState(), not by copying bytes. */
struct CompanionState {
  uint32_t total_xp = 0;
  uint32_t valid_pages = 0;
  uint32_t last_read_unix = 0;
  uint32_t achievement_bits = 0;
  int32_t last_read_day = -1; ///< Local civil day; valid only with a trusted clock.
  int32_t last_qualified_day = -1;
  uint16_t streak_days = 0;
  uint16_t session_pages = 0;
  uint16_t day_pages = 0;
  CharacterClass character_class = CharacterClass::Wanderer;
  uint8_t equipment_id = 0;
};

/** @brief Confirmed forward page change supplied by the firmware adapter. */
struct PageTurnEvent {
  uint32_t book_id;       ///< Stable hash supplied by the reader adapter.
  uint32_t page_index;    ///< Logical rendered page, not a button press count.
  uint32_t monotonic_ms;  ///< Milliseconds since boot; wraps naturally.
  uint32_t unix_seconds;  ///< Zero if wall clock is unavailable.
  int32_t local_day;      ///< Local civil day number, e.g. days since Unix epoch.
  bool clock_trusted;
  uint8_t local_hour;     ///< 0-23, or 255 when unknown.
  uint8_t weekday;        ///< 0=Monday .. 6=Sunday, or 255 when unknown.
};

}
