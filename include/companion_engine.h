#pragma once

#include "companion_state.h"
#include "state_codec.h"

#include <cstdint>

namespace companion {

/** @brief Allocation-free gameplay state machine for confirmed page changes. */
class CompanionEngine {
 public:
  static constexpr uint32_t kMinPageSeconds = 15;
  static constexpr uint32_t kMaxPageSeconds = 300;
  static constexpr uint32_t kFlushIntervalMs = 600000;
  static constexpr size_t kRecordSize = kStateRecordSize;

  /** @brief Credit an eligible forward page after the initial anchor event.
   *  @return True when a page earned XP.
   */
  bool onPageTurn(const PageTurnEvent& event);
  /** @brief Reset the page anchor and session count when reading begins.
   * Pass the current monotonic time so a changed session count can be flushed.
   */
  void beginSession(uint32_t monotonic_ms = 0);
  /** @brief Read the current gameplay state. */
  const CompanionState& state() const { return state_; }
  /** @brief Replace state after a trusted load and reset runtime timers. */
  void setState(const CompanionState& state);
  /** @brief Indicate whether gameplay changes still need persistence. */
  bool dirty() const { return dirty_; }
  /** @brief Check whether ten minutes elapsed since the first unsaved page. */
  bool flushDue(uint32_t monotonic_ms) const;
  /** @brief Clear the dirty flag after a successful storage commit. */
  void markFlushed();

  /** @brief Encode state for storage at a safe lifecycle boundary. */
  bool encode(uint8_t* out, size_t length) const;
  /** @brief Load a validated record, leaving state unchanged on failure. */
  bool decode(const uint8_t* data, size_t length);

 private:
  CompanionState state_{};
  uint32_t anchor_book_ = 0;
  uint32_t anchor_page_ = 0;
  uint32_t anchor_ms_ = 0;
  uint32_t first_dirty_ms_ = 0;
  bool has_anchor_ = false;
  bool dirty_ = false;
};

}
