#pragma once

#include "reassembler.hh"

namespace blastoise {
namespace reassembler {

/// This reassembler forwards anything after the most recent sequence
class MostRecentReassembler final : public Reassembler {
public:
  ForwardResult handle_reliable(Packet packet,
                                PacketSequence last_forwarded) override;

  ForwardResult handle_unreliable(Packet packet, PacketSequence last_forwarded,
                                  PacketSequence last_reliable) override;
};
} // namespace reassembler
} // namespace blastoise