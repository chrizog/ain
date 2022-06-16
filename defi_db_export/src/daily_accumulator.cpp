#include "daily_accumulator.h"
#include "export_utils.h"
#include <sstream>
#include <string>

namespace defi_export {

bool DayAccumulator::update(const std::uint64_t timestamp, const double ratio)
{
    bool result = false;
    auto day = helper::timestamp_to_year_month_day(timestamp);

    if (first) {
        first = false;
        current.high = ratio;
        current.low = ratio;
    } else if (day > current.day) {
        result = true;
        last = current;
        current.high = ratio;
        current.low = ratio;
    } else {
        if (ratio > current.high) {
            current.high = ratio;
        }
        if (ratio < current.low) {
            current.low = ratio;
        }
    }

    current.day = day;
    return result;
}

} // namespace defi_export
