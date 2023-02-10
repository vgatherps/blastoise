#pragma once

#include "packet.hh"

#include <memory>
#include <variant>

namespace blastoise {
namespace reassembler {

struct BulkForward {
  Packet head;
  std::vector<Packet> buffered;
};

struct Forward {
  Packet packet;
};

struct SkipPacket {};

using ForwardResult = std::variant<SkipPacket, Forward, BulkForward>;

/// This interface describes something that reassembles a single stream
/// from a reliable/ordered stream and an unreliable/unordered stream
class Reassembler {

public:
  virtual ForwardResult handle_reliable(Packet p,
                                        PacketSequence last_forwarded) = 0;

  virtual ForwardResult handle_unreliable(Packet packet,
                                          PacketSequence last_forwarded,
                                          PacketSequence last_reliable) = 0;

  virtual ~Reassembler() {}
};

enum class ForwarderType { FullyOrdered, ForwardAll, MostRecent };

std::unique_ptr<Reassembler> create_reassembler(ForwarderType);

} // namespace reassembler
} // namespace blastoise