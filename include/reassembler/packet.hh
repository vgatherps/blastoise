#pragma once

#include "protocol/header.hh"

#include <cstdint>
#include <span>
#include <vector>

namespace blastoise {
namespace reassembler {

using protocol::PacketHash;
using protocol::PacketSeqNo;
using protocol::PacketSequence;

template <class Storage> struct Packet {
  PacketSequence sequence;
  Storage data;

  Packet(PacketSequence seq, Storage data)
      : sequence(seq), data(std::move(data)) {}

  bool operator==(const Packet<Storage> &other) const = default;
};

} // namespace reassembler
} // namespace blastoise