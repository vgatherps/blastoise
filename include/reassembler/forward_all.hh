#pragma once

#include "reassembler.hh"

// stl implementation is quite slow but that's irrelevant for a hobby project
#include <unordered_set>

namespace blastoise {
namespace reassembler {

/// This reassembler forwards one copy of every received packet, in
/// arbitrary order
class ForwardAllReassembler final : public Reassembler {

  std::unordered_set<PacketSeqNo> unconfirmed_packets;

public:
  ForwardResult handle_reliable(Packet packet,
                                PacketSequence last_forwarded) override;

  ForwardResult handle_unreliable(Packet packet, PacketSequence last_forwarded,
                                  PacketSequence last_reliable) override;
};
} // namespace reassembler
} // namespace blastoise