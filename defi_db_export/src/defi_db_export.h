#ifndef DEFI_EXPORT_H
#define DEFI_EXPORT_H

#include "daily_accumulator.h"
#include <cstdint>
#include <string>

namespace defi_export {

class DefiPriceExport {

public:
  struct ConstructionToken {
    std::string db_host;
    std::string db_user;
    std::string db_pwd;
    std::string db_name;
  };

  DefiPriceExport(const ConstructionToken token) : token(token){};

  bool export_price(std::int64_t timestamp, std::int64_t idA, std::int64_t idB,
                    std::int64_t reserveA, std::int64_t reserveB);

private:
  struct PriceData {
    std::string date_str;
    std::int64_t idA;
    std::int64_t idB;
    float high;
    float low;
  };

  void insert_data(const PriceData &price_data);
  std::string create_map_key(const std::int64_t idA, std::int64_t idB) const;

  ConstructionToken token;
  AccumulatorMap<DayAccumulator> acc_map;

};

} // namespace defi_export

#endif /* DEFI_EXPORT_H */
