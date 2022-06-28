#include "block_accumulator.h"
#include <sstream>
#include <string>

namespace defi_export {

bool BlockAccumulator::update(const std::uint64_t block_height, const std::uint64_t reserveA, const std::uint64_t reserveB)
{
    bool result = false;

    if (first) {
        first = false;
        current.reserveA = reserveA;
        current.reserveB = reserveB;
    } else if (block_height > current.block_height) {
        result = true;
        last = current;
        current.reserveA = reserveA;
        current.reserveB = reserveB;
    } else {
        current.reserveA = reserveA;
        current.reserveB = reserveB;
    }

    current.block_height = block_height;
    return result;
}

} // namespace defi_export
