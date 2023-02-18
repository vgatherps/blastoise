#include "receiver/decode_packet.hh"
#include <cstring>

namespace blastoise::receiver {
using protocol::PacketHeader;
SplitResult split_packet(seastar::net::packet data) {
  if (data.len() < sizeof(PacketHeader)) {
    return NotEnoughBytes{std::move(data)};
  }

  PacketHeader header;
  std::memcpy(&header, data.get_header<PacketHeader>(), sizeof(PacketHeader));

  if (header.length < (data.len() - sizeof(PacketHeader))) {
    return NotEnoughBytes{std::move(data)};
  }

  data.trim_front(sizeof(PacketHeader));

  std::optional<seastar::net::packet> rest;

  if (data.len() > header.length) {
    std::size_t remaining = data.len() - header.length;
    // This is somewhat less efficient than a single split,
    // but it is MUCH easier to implement,
    // and in practice individual messages will be small
    // and not cross tons of fragments
    rest = data.share(header.length, remaining);
    data.retain_front(header.length);
  }

  assert(data.len() == header.length);

  return SplitPacket{.header = header,
                     .packet_data = std::move(data),
                     .rest = std::move(rest)};
}

protocol::StreamKey decode_packet(const protocol::PacketHeader &header,
                                  seastar::net::packet &data,
                                  protocol::PacketMask mask,
                                  protocol::DecoderMap decoders) {
  protocol::DecodeData decoder = decoders[header.msg_type];
  if (!decoder.valid()) {
    throw std::runtime_error("Invalid message type received");
  }

  std::uint8_t *data_ptr = data.get_header<std::uint8_t>(header.length);

  std::span<std::uint8_t> data_span(data_ptr, header.length);

  protocol::mask_in_place(data_span, mask);

  protocol::HashedStreamKey hashed = decoder.key_decoder(data_span);

  if (hashed.compress() != header.checksum) {
    // should log and drop, this could theoretically happen
    // on restarts I think if somehow somewhere
    // a udp packet is exceptionally delayed and we get it after a restart
    throw std::runtime_error("Incoming message did not pass checksum");
  }

  return hashed.key;
}

} // namespace blastoise::receiver
