#include "reassembler/forward_all.hh"
#include "gtest/gtest.h"

#include "reassembler_test_common.hh"

TEST(ForwardAllForwardsOneUnreliable, ForwardAllTests) {
  Packet packet = make_packet(123, {1, 2, 3, 4, 5});

  ForwardAllReassembler reass;

  ASSERT_EQ(reass.handle_unreliable(packet.clone(), make_sequence(102),
                                    make_sequence(112)),
            forward(packet.clone()));
}

TEST(ForwardAllForwardsOneReliable, ForwardAllTests) {
  Packet packet = make_packet(123, {1, 2, 3, 4, 5});

  ForwardAllReassembler reass;

  ASSERT_EQ(reass.handle_reliable(packet.clone(), make_sequence(102)),
            forward(packet.clone()));
}

TEST(ForwardAllForwardsBehindUneliable, ForwardAllTests) {
  Packet packet = make_packet(123, {1, 2, 3, 4, 5});

  ForwardAllReassembler reass;

  ASSERT_EQ(reass.handle_unreliable(packet.clone(), make_sequence(143),
                                    make_sequence(112)),
            forward(packet.clone()));
}

TEST(ForwardAllForwardsBehindReliable, ForwardAllTests) {
  Packet packet = make_packet(123, {1, 2, 3, 4, 5});

  ForwardAllReassembler reass;

  ASSERT_EQ(reass.handle_reliable(packet.clone(), make_sequence(124)),
            forward(packet.clone()));
}