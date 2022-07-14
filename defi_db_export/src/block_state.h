#ifndef DEFI_BLOCK_ACCUMULATOR_H
#define DEFI_BLOCK_ACCUMULATOR_H

#include "date.h"
#include <map>

namespace defi_export {

class BlockState {
public:
  struct Block {
  std::uint64_t block_height;
  std::uint64_t reserve_a;
  std::uint64_t reserve_b;
  };

  bool update(const std::uint64_t block_height, const std::uint64_t reserve_a, const std::uint64_t reserve_b);
  Block get_last_block() const { return last; }

private:
  Block current{};
  Block last{};
  bool first{true};
};


} // namespace defi_export

#endif /* DEFI_BLOCK_ACCUMULATOR_H */