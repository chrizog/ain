#ifndef DEFI_DAILY_ACCUMULATOR_H
#define DEFI_DAILY_ACCUMULATOR_H

#include "date.h"
#include <map>

namespace defi_export {

struct DailyRecord {
  date::year_month_day day;
  float high;
  float low;
};

class DayAccumulator {
public:
  bool update(const std::uint64_t timestamp, const double ratio);
  DailyRecord get_last_record() const { return last; }
  DailyRecord get_current_record() const { return current; }

private:
  DailyRecord current{};
  DailyRecord last{};
  bool first{true};
};

template <typename T> class AccumulatorMap {
public:
  T &get_accumulator(const std::string &key) {
    auto search = accumulators.find(key);
    if (search == accumulators.end()) {
      T acc_{};
      accumulators.insert(std::make_pair(key, acc_));
    }
    return accumulators[key];
  };

private:
  std::map<std::string, T> accumulators;
};

} // namespace defi_export

#endif /* DEFI_DAILY_ACCUMULATOR_H */