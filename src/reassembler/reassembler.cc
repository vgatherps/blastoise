#include "reassembler/reassembler.hh"
#include "reassembler/forward_all.hh"
#include "reassembler/most_recent.hh"

namespace blastoise {
namespace reassembler {

std::unique_ptr<Reassembler> create_reassembler(ForwarderType type) {
  switch (type) {
  case ForwarderType::MostRecent:
    return std::make_unique<MostRecentReassembler>();
  case ForwarderType::ForwardAll:
    return std::make_unique<ForwardAllReassembler>();
  case ForwarderType::FullyOrdered:
    throw "blah";
  }
}

} // namespace reassembler
} // namespace blastoise