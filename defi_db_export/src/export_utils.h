#ifndef EXPORT_UTILS_H
#define EXPORT_UTILS_H

#include "date.h"
#include <string>

namespace defi_export {
namespace helper {

date::year_month_day timestamp_to_year_month_day(const std::uint64_t timestamp);
std::string year_month_day_to_string(const date::year_month_day &ymd);

} // namespace helper
} // namespace defi_export

#endif