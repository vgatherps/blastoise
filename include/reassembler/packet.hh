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

  bool operator==(const PacketSequence &other) const = default;
};

template <class Storage> struct Packet {
  PacketSequence sequence;
  Storage data;

  Packet(PacketSequence seq, std::vector<std::uint8_t> data)
      : sequence(seq), data(std::move(data)) {}

  bool operator==(const Packet<Storage> &other) const = default;
};

} // namespace reassembler
} // namespace blastoise