#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace blastoise {
namespace reassembler {

using PacketSeqNo = std::uint32_t;
using PacketHash = std::uint32_t;

struct PacketSequence {
  PacketSeqNo sequence;
  PacketHash hash;

  bool ahead_of(const PacketSequence &other) const {
    return sequence > other.sequence;
  }

  bool implies_already_seen(const PacketSequence &other) const {
    return !other.ahead_of(*this);
  }

  bool succeeds(const PacketSequence &other) const {
    return sequence == (other.sequence + 1);
  }
};

struct Packet {
  PacketSequence sequence;
  std::vector<std::uint8_t> data;

  Packet() = delete;

  Packet(Packet &&) = default;
  Packet &operator=(Packet &&) = default;

  Packet(const Packet &p) = delete;
  Packet &operator=(const Packet &p) = delete;
};

} // namespace reassembler
} // namespace blastoise