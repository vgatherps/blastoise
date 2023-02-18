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
    // and in practice this retain_front loop will never have more
    // than one iteration as we've already linearized the first fragment
    // to contain header.length
    rest = data.share(header.length, remaining);
    data.retain_front(header.length);
  }

  seastar::temporary_buffer<char> linearized_data;

  assert(data.len() == header.length);

  return SplitPacket{.header = header,
                     .packet_data = std::move(data),
                     .rest = std::move(rest)};
}
} // namespace blastoise::receiver
