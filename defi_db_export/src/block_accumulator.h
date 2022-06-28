#ifndef DEFI_BLOCK_ACCUMULATOR_H
#define DEFI_BLOCK_ACCUMULATOR_H

#include "date.h"
#include <map>

namespace defi_export {

struct BlockRecord {
  std::uint64_t block_height;
  std::uint64_t reserveA;
  std::uint64_t reserveB;
};

class BlockAccumulator {
public:
  bool update(const std::uint64_t block_height, const std::uint64_t reserveA, const std::uint64_t reserveB);
  BlockRecord get_last_record() const { return last; }
  BlockRecord get_current_record() const { return current; }

private:
  BlockRecord current{};
  BlockRecord last{};
  bool first{true};
};


} // namespace defi_export

#endif /* DEFI_BLOCK_ACCUMULATOR_H */