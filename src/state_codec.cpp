#include "state_codec.h"

#include <cstring>

namespace companion {
namespace {

constexpr uint8_t kVersion = 1;

void put16(uint8_t* p, uint16_t value) {
  p[0] = static_cast<uint8_t>(value);
  p[1] = static_cast<uint8_t>(value >> 8);
}

void put32(uint8_t* p, uint32_t value) {
  for (unsigned i = 0; i < 4; ++i) p[i] = static_cast<uint8_t>(value >> (8u * i));
}

uint16_t get16(const uint8_t* p) {
  return static_cast<uint16_t>(p[0] | (static_cast<uint16_t>(p[1]) << 8));
}

uint32_t get32(const uint8_t* p) {
  return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
         (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
}

uint16_t crc16(const uint8_t* data, size_t size) {
  uint16_t crc = 0xffff;
  for (size_t i = 0; i < size; ++i) {
    crc ^= static_cast<uint16_t>(data[i]) << 8;
    for (unsigned bit = 0; bit < 8; ++bit)
      crc = static_cast<uint16_t>((crc << 1) ^ ((crc & 0x8000u) ? 0x1021u : 0u));
  }
  return crc;
}

}

bool encodeState(const CompanionState& state, uint8_t* out, size_t length) {
  if (!out || length < kStateRecordSize) return false;
  std::memset(out, 0, kStateRecordSize);
  out[0] = 'C';
  out[1] = 'P';
  out[2] = kVersion;
  put32(out + 4, state.total_xp);
  put32(out + 8, state.valid_pages);
  put32(out + 12, state.last_read_unix);
  put32(out + 16, state.achievement_bits);
  put32(out + 20, static_cast<uint32_t>(state.last_read_day));
  put32(out + 24, static_cast<uint32_t>(state.last_qualified_day));
  put16(out + 28, state.streak_days);
  put16(out + 30, state.session_pages);
  put16(out + 32, state.day_pages);
  out[34] = static_cast<uint8_t>(state.character_class);
  out[35] = state.equipment_id;
  put16(out + 36, crc16(out, 36));
  return true;
}

bool decodeState(const uint8_t* data, size_t length, CompanionState& out) {
  if (!data || length != kStateRecordSize || data[0] != 'C' || data[1] != 'P' ||
      data[2] != kVersion || data[3] != 0 || get16(data + 36) != crc16(data, 36) ||
      data[34] > static_cast<uint8_t>(CharacterClass::Archivist))
    return false;
  CompanionState loaded;
  loaded.total_xp = get32(data + 4);
  loaded.valid_pages = get32(data + 8);
  loaded.last_read_unix = get32(data + 12);
  loaded.achievement_bits = get32(data + 16);
  loaded.last_read_day = static_cast<int32_t>(get32(data + 20));
  loaded.last_qualified_day = static_cast<int32_t>(get32(data + 24));
  loaded.streak_days = get16(data + 28);
  loaded.session_pages = get16(data + 30);
  loaded.day_pages = get16(data + 32);
  loaded.character_class = static_cast<CharacterClass>(data[34]);
  loaded.equipment_id = data[35];
  out = loaded;
  return true;
}

}
