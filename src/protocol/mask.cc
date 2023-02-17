#include "protocol/mask.hh"

#include <cstring>

namespace blastoise::net {

void mask_in_place(std::span<std::uint8_t> bytes, PacketMask mask) {
  constexpr std::size_t MASK_SIZE = mask.size();
  std::size_t at = 0;
  std::uint32_t maski;
  std::memcpy(&maski, &mask, MASK_SIZE);
  for (; at + MASK_SIZE < bytes.size(); at += MASK_SIZE) {
    std::uint32_t arr_data;
    std::memcpy(&arr_data, &bytes[at], MASK_SIZE);
    arr_data ^= maski;
    std::memcpy(&bytes[at], &arr_data, MASK_SIZE);
  }

  for (; at < bytes.size(); at++) {
    bytes[at] ^= mask[at % MASK_SIZE];
  }
}
} // namespace blastoise::net