#pragma once

#include <cstdint>

namespace blastoise::protocol {

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

struct PacketHeader {
  std::uint32_t length;
  PacketSeqNo sequence;
  PacketHash checksum;
  std::uint8_t sender_id;
  std::uint8_t msg_type;

  PacketSequence packet_sequence() const {
    return PacketSequence{.sequence = sequence, .hash = checksum};
  }
} __attribute__((packed));

static_assert(sizeof(PacketHeader) == 14, "Packet header has wrong size");
static_assert(alignof(PacketHeader) == 1,
              "Packet header has nontrivial padding/alignment");

} // namespace blastoise::protocol