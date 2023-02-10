#pragma once

#include "reassembler.hh"

// stl implementation is quite slow but that's irrelevant for a hobby project
#include <deque>

namespace blastoise {
namespace reassembler {

class FullyOrderedReassembler final : public Reassembler {

  struct PendingPacketBatch {
    std::vector<Packet> packets;
  };

  std::deque<PendingPacketBatch> packet_batches;

  ForwardResult forward_packet(Packet packet);
  void insert_pending(Packet packet);

public:
  ForwardResult handle_reliable(Packet packet,
                                PacketSequence last_forwarded) override;

  ForwardResult handle_unreliable(Packet packet, PacketSequence last_forwarded,
                                  PacketSequence last_reliable) override;
};
} // namespace reassembler
} // namespace blastoise