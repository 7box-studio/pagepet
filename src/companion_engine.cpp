#include "companion_engine.h"

#include <limits>

namespace companion {
namespace {

constexpr uint16_t kPagesForDay = 5;

template <typename T> void incrementUnlessMax(T& value) {
  if (value < std::numeric_limits<T>::max()) ++value;
}

}

void CompanionEngine::beginSession(uint32_t monotonic_ms) {
  has_anchor_ = false;
  if (state_.session_pages != 0) {
    state_.session_pages = 0;
    if (!dirty_) first_dirty_ms_ = monotonic_ms;
    dirty_ = true;
  }
}

void CompanionEngine::setState(const CompanionState& state) {
  state_ = state;
  has_anchor_ = false;
  dirty_ = false;
  first_dirty_ms_ = 0;
}

bool CompanionEngine::onPageTurn(const PageTurnEvent& event) {
  // A redraw of the current logical page is not a new reading anchor.
  if (has_anchor_ && event.book_id == anchor_book_ &&
      event.page_index == anchor_page_) return false;
  const bool eligible = has_anchor_ && event.book_id == anchor_book_ &&
                        anchor_page_ != UINT32_MAX &&
                        event.page_index == anchor_page_ + 1 &&
                        static_cast<uint32_t>(event.monotonic_ms - anchor_ms_) >=
                            kMinPageSeconds * 1000u &&
                        static_cast<uint32_t>(event.monotonic_ms - anchor_ms_) <=
                            kMaxPageSeconds * 1000u;
  anchor_book_ = event.book_id;
  anchor_page_ = event.page_index;
  anchor_ms_ = event.monotonic_ms;
  has_anchor_ = true;
  if (!eligible) return false;

  if (!dirty_) first_dirty_ms_ = event.monotonic_ms;
  dirty_ = true;
  incrementUnlessMax(state_.valid_pages);
  incrementUnlessMax(state_.session_pages);

  if (event.clock_trusted && event.unix_seconds != 0) {
    if (event.unix_seconds >= state_.last_read_unix)
      state_.last_read_unix = event.unix_seconds;
    if (event.local_day >= 0 && event.local_day >= state_.last_read_day) {
      if (event.local_day != state_.last_read_day) {
        state_.last_read_day = event.local_day;
        state_.day_pages = 0;
      }
      incrementUnlessMax(state_.day_pages);
      if (state_.day_pages >= kPagesForDay &&
          state_.last_qualified_day != event.local_day) {
        state_.streak_days =
            (state_.last_qualified_day == event.local_day - 1)
                ? static_cast<uint16_t>(state_.streak_days == UINT16_MAX
                                            ? UINT16_MAX
                                            : state_.streak_days + 1)
                : 1;
        state_.last_qualified_day = event.local_day;
      }
    }
    if (event.local_hour < 5 || (event.local_hour >= 22 && event.local_hour < 24))
      state_.achievement_bits |= NightReader;
    if (event.weekday == 5 || event.weekday == 6)
      state_.achievement_bits |= WeekendReader;
  }
  if (state_.session_pages >= 100) state_.achievement_bits |= Marathon;

  const uint16_t streak_bonus = state_.streak_days > 10 ? 10 : state_.streak_days;
  const uint32_t earned_xp = 10u + (streak_bonus + 1u) / 2u;
  if (state_.total_xp > UINT32_MAX - earned_xp)
    state_.total_xp = UINT32_MAX;
  else
    state_.total_xp += earned_xp;
  return true;
}

bool CompanionEngine::flushDue(uint32_t monotonic_ms) const {
  return dirty_ &&
         static_cast<uint32_t>(monotonic_ms - first_dirty_ms_) >= kFlushIntervalMs;
}

void CompanionEngine::markFlushed() {
  dirty_ = false;
  first_dirty_ms_ = 0;
}

bool CompanionEngine::encode(uint8_t* out, size_t length) const {
  return encodeState(state_, out, length);
}

bool CompanionEngine::decode(const uint8_t* data, size_t length) {
  CompanionState loaded;
  if (!decodeState(data, length, loaded)) return false;
  setState(loaded);
  return true;
}

}
