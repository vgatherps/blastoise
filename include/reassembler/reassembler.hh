#pragma once

#include "packet.hh"
#include "protocol/keys.hh"

#include <memory>
#include <variant>

namespace blastoise {
namespace reassembler {

template <class Storage> struct BulkForward {
  Packet<Storage> head;
  std::vector<Packet<Storage>> buffered;
  bool operator==(const BulkForward &o) const = default;
};

template <class Storage> struct Forward {
  Packet<Storage> packet;

  bool operator==(const Forward &o) const = default;
};

struct SkipPacket {
  bool operator==(const SkipPacket &o) const = default;
};

template <class Storage>
using ForwardResult =
    std::variant<SkipPacket, Forward<Storage>, BulkForward<Storage>>;

template <class Storage> class Reassembler {
public:
  virtual ForwardResult<Storage> handle_reliable(Packet<Storage> p) = 0;
  virtual ForwardResult<Storage> handle_unreliable(Packet<Storage> p) = 0;
  virtual ~Reassembler() {}
};

template <class Storage>
std::unique_ptr<Reassembler<Storage>> create_reassembler(protocol::StreamType);

} // namespace reassembler
} // namespace blastoise