#include "reassembler/reassembler.hh"
#include "reassembler/forward_all.hh"
#include "reassembler/fully_ordered.hh"
#include "reassembler/most_recent.hh"

namespace blastoise {
namespace reassembler {

template <class T>
std::unique_ptr<Reassembler<T>> do_create_reassembler(ForwarderType type) {
  switch (type) {
  case ForwarderType::MostRecent:
    return std::make_unique<MostRecentReassembler<T>>();
  case ForwarderType::ForwardAll:
    return std::make_unique<ForwardAllReassembler<T>>();
  case ForwarderType::FullyOrdered:
    return std::make_unique<FullyOrderedReassembler<T>>();
  }
}

} // namespace reassembler
} // namespace blastoise