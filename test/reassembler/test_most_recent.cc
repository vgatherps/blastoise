#include "reassembler/most_recent.hh"
#include "gtest/gtest.h"

#include "reassembler_test_common.hh"

TEST(MostRecentForwardsOneUnreliable, MostRecentTests) {
  Packet packet = make_packet(123, {1, 2, 3, 4, 5});

  MostRecentReassembler reass;

  ASSERT_EQ(reass.handle_unreliable(packet.clone(), make_sequence(102),
                                    make_sequence(112)),
            forward(packet.clone()));
}

TEST(MostRecentForwardsOneReliable, MostRecentTests) {
  Packet packet = make_packet(123, {1, 2, 3, 4, 5});

  MostRecentReassembler reass;

  ASSERT_EQ(reass.handle_reliable(packet.clone(), make_sequence(102)),
            forward(packet.clone()));
}

TEST(MostRecentIgnoresOldUneliable, MostRecentTests) {
  Packet packet = make_packet(123, {1, 2, 3, 4, 5});

  MostRecentReassembler reass;

  ASSERT_EQ(reass.handle_unreliable(packet.clone(), make_sequence(143),
                                    make_sequence(112)),
            skip());
}

TEST(MostRecentIgnoresOldReliable, MostRecentTests) {
  Packet packet = make_packet(123, {1, 2, 3, 4, 5});

  MostRecentReassembler reass;

  ASSERT_EQ(reass.handle_reliable(packet.clone(), make_sequence(124)), skip());
}