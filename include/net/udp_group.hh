#pragma once

#include "net/udp_socket.hh"
#include "protocol/client.hh"

#include <seastar/core/pipe.hh>

#include <unordered_map>
#include <vector>

namespace blastoise::net {
class UdpGroup {
  std::unordered_map<protocol::ClientId, UdpSocket> sockets;

public:
  void send_to_all(UdpPacket &packet);
};
} // namespace blastoise::net