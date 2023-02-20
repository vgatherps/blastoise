#pragma once

#include "protocol/keys.hh"
#include "reassembler/reassembler.hh"

#include <seastar/net/packet.hh>

#include <unordered_map>

namespace blastoise::receiver {

using PacketReassembler = reassembler::Reassembler<seastar::net::packet>;

struct StreamId {
  protocol::StreamKey key;
  std::uint8_t sender_id;

  bool operator==(const StreamId &other) const = default;
};
} // namespace blastoise::receiver

namespace std {
using namespace blastoise;
using namespace blastoise::receiver;
template <> struct hash<StreamId> {
  std::size_t operator()(const StreamId &id) const {
    return std::hash<protocol::StreamKey>{}(id.key) ^
           std::hash<std::uint8_t>{}(id.sender_id);
  }
};
} // namespace std

namespace blastoise::receiver {
class StreamManager {

  std::unordered_map<StreamId, std::unique_ptr<PacketReassembler>>
      stream_reassemblers;

public:
  reassembler::ForwardResult<seastar::net::packet>
  handle_reliable_packet(const protocol::PacketHeader &header,
                         protocol::StreamKey key, seastar::net::packet data);

  reassembler::ForwardResult<seastar::net::packet>
  handle_unreliable_packet(const protocol::PacketHeader &header,
                           protocol::StreamKey key, seastar::net::packet data);
};

} // namespace blastoise::receiver