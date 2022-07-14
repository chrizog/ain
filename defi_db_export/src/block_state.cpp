#include "block_state.h"
#include <sstream>
#include <string>

namespace defi_export {

bool BlockState::update(const std::uint64_t block_height, const std::uint64_t reserve_a, const std::uint64_t reserve_b)
{
    bool is_next_block = false;

    if (first) {
        first = false;
        current.reserve_a = reserve_a;
        current.reserve_b = reserve_b;
    } else if (block_height > current.block_height) {
        is_next_block = true;
        last = current;
        current.reserve_a = reserve_a;
        current.reserve_b = reserve_b;
    } else {
        current.reserve_a = reserve_a;
        current.reserve_b = reserve_b;
    }

    current.block_height = block_height;
    return is_next_block;
}

} // namespace defi_export
