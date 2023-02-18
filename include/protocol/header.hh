#pragma once

#include <cstdint>

namespace blastoise::protocol {

using PacketSeqNo = std::uint32_t;
using PacketHash = std::uint16_t;

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

struct PacketHeader {
  std::uint32_t length;
  PacketSeqNo sequence;
  PacketHash checksum;
  std::uint8_t sender_id;
  std::uint8_t msg_type;

  PacketSequence packet_sequence() const {
    return PacketSequence{.sequence = sequence, .hash = checksum};
  }
};

static_assert(sizeof(PacketHeader) == 12, "Packet header has wrong size");

} // namespace blastoise::protocol