#include "export_utils.h"
#include <sstream>
#include <chrono>

namespace defi_export {
namespace helper {

date::year_month_day
timestamp_to_year_month_day(const std::uint64_t timestamp) {
  const auto duration_seconds_ = std::chrono::seconds(timestamp);
  const auto time_point_ =
      std::chrono::time_point<std::chrono::system_clock>(duration_seconds_);
  return date::year_month_day{date::floor<date::days>(time_point_)};
}

std::string year_month_day_to_string(const date::year_month_day &ymd) {
  std::stringstream ss;
  ss << ymd;
  return ss.str();
}

} // namespace helper
} // namespace defi_export