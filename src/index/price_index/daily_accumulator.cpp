#include "daily_accumulator.h"
#include "logging.h"
#include <sstream>
#include <string>

namespace price_index {

namespace helper {

date::year_month_day timestamp_to_year_month_day(const std::uint64_t timestamp)
{
    const auto duration_seconds_ = std::chrono::seconds(timestamp);
    const auto time_point_ =
        std::chrono::time_point<std::chrono::system_clock>(duration_seconds_);
    return date::year_month_day{date::floor<date::days>(time_point_)};
}

std::string year_month_day_to_string(const date::year_month_day& ymd) {
    std::stringstream ss;
    ss << ymd;
    return ss.str();
}

} // namespace helper


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


DayAccumulator& DayAccumulatorMap::get_accumulator(const std::int64_t id_token_a, const std::int64_t id_token_b) {
    std::string key = std::to_string(id_token_a) + "-" + std::to_string(id_token_b);
    auto search = accumulators.find(key);
    if (search == accumulators.end()) {
        DayAccumulator acc_{};
        accumulators.insert(std::make_pair(key, acc_));
    }
    return accumulators[key];
}

} // namespace price_index
