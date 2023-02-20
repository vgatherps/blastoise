#include "receiver/stream_manager.hh"

namespace blastoise::receiver {
reassembler::ForwardResult<seastar::net::packet>
StreamManager::handle_reliable_packet(const protocol::PacketHeader &header,
                                      protocol::StreamKey key,
                                      seastar::net::packet data) {
  StreamId id{.key = key, .sender_id = header.sender_id};
  auto it = stream_reassemblers.find(id);
  if (it == stream_reassemblers.end()) {
    auto reassembler =
        reassembler::create_reassembler<seastar::net::packet>(key.type);
    it = stream_reassemblers.emplace(id, std::move(reassembler)).first;
  }

  protocol::PacketSequence packet_seq{.sequence = header.sequence,
                                      .hash = header.checksum};

  reassembler::Packet p(packet_seq, std::move(data));

  return it->second->handle_reliable(std::move(p));
}

reassembler::ForwardResult<seastar::net::packet>
StreamManager::handle_unreliable_packet(const protocol::PacketHeader &header,
                                        protocol::StreamKey key,
                                        seastar::net::packet data) {
  StreamId id{.key = key, .sender_id = header.sender_id};
  auto it = stream_reassemblers.find(id);
  if (it == stream_reassemblers.end()) {
    return reassembler::SkipPacket{};
  }

  protocol::PacketSequence packet_seq{.sequence = header.sequence,
                                      .hash = header.checksum};

  reassembler::Packet p(packet_seq, std::move(data));

  return it->second->handle_unreliable(std::move(p));
}

} // namespace blastoise::receiver