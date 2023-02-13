#include "net/udp_packet.hh"

namespace blastoise::net {

UdpPacket::UdpPacket(std::span<std::uint8_t> data)
    : underlying(data.data(), data.size()) {}

} // namespace blastoise::net