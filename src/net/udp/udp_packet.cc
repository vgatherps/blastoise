#include "net/udp/udp_packet.hh"

namespace blastoise::net {

UdpPacket::UdpPacket(std::span<char> data)
    : underlying(data.data(), data.size()),
      del(seastar::make_object_deleter(underlying.share())) {}

seastar::net::packet UdpPacket::create_packet() {
  seastar::net::fragment frag{.base = underlying.get_write(),
                              .size = underlying.size()};

  return seastar::net::packet(frag, del.share());
}

} // namespace blastoise::net