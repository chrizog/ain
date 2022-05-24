#ifndef DEFI_PRICE_INDEX_DAILY_ACC_H
#define DEFI_PRICE_INDEX_DAILY_ACC_H

#include "date.h"

#include <map>
#include <string>

namespace price_index {

namespace helper {

date::year_month_day timestamp_to_year_month_day(const std::uint64_t timestamp);
std::string year_month_day_to_string(const date::year_month_day& ymd);

} // namespace helper


struct DailyRecord {
    date::year_month_day day;
    float high;
    float low;
};

class DayAccumulator
{
public:
    bool update(const std::uint64_t timestamp, const double ratio);
    DailyRecord get_last_record() const { return last; }
    DailyRecord get_current_record() const { return current; }

private:
    DailyRecord current{};
    DailyRecord last{};
    bool first{true};
};

class DayAccumulatorMap
{
public:
    DayAccumulator& get_accumulator(const std::int64_t id_token_a, const std::int64_t id_token_b);

private:
    std::map<std::string, DayAccumulator> accumulators;
};


} // namespace price_index


#endif // DEFI_PRICE_INDEX_DAILY_ACC_H