#pragma once

#include <array>
#include <cstdint>
#include <span>

namespace blastoise::protocol {

using PacketMask = std::array<std::uint8_t, 4>;

void mask_in_place(std::span<std::uint8_t>, PacketMask mask);

} // namespace blastoise::protocol