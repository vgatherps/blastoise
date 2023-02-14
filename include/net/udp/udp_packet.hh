#pragma once

#include "seastar/net/packet.hh"
#include <seastar/core/temporary_buffer.hh>

#include <span>

namespace blastoise::net {
// Seastar has this sort of wacky packet fragment api,
// and as far as I can tell, if you use it *just right*,
// it's zero copy and you only copy/allocate to the
// extend you need the packet itself to allocate fragment tracking.
//
// This is all undocumented so maybe there's a better way,
// this feels *so* hacky just to avoid seastar allocating 100 copies
// of one of my packets.
//
// It's potentially because I'm trying to fire-and-forget, so I want nontrivial
// deletion. Maybe in the real world, you only have trivial deletes because
// you're expected to always get and await? As far as I can tell, the best thing
// to do is
// 1. hold a shared pointer to the underlying data
// 2. copy this shared pointer into the deleter
// 3. The deleter decrements the count every thing time
//
// I *think* that using temporary buffer is the way to go here, instead of
// trying to do it myself? I *think* that does what I want?
class UdpPacket {
  seastar::temporary_buffer<char> underlying;
  seastar::deleter del;
  seastar::net::fragment frag;

public:
  // TODO make this take a deleter allowing truer copyless/allocationless IO?
  UdpPacket(std::span<char> data);

  UdpPacket(UdpPacket &&) = default;
  UdpPacket &operator=(UdpPacket &&) = default;

  UdpPacket(const UdpPacket &other) = delete;
  UdpPacket &operator=(const UdpPacket &) = delete;

  seastar::net::packet create_packet();
};
} // namespace blastoise::net