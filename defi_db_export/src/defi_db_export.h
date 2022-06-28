#ifndef DEFI_EXPORT_H
#define DEFI_EXPORT_H

#include "block_accumulator.h"
#include "daily_accumulator.h"
#include <cstdint>
#include <string>
#include <vector>

namespace defi_export {

class DefiBlockTimestampExport {
public:
  struct ConstructionToken {
    std::string db_host;
    std::string db_user;
    std::string db_pwd;
    std::string db_name;
  };

  DefiBlockTimestampExport(const ConstructionToken token) : token(token){};
  void export_block(std::int64_t block_height, std::int64_t timestamp);
  void remove_blocks(std::int64_t block_height_start_remove);

private:
  struct BlockTimestampData {
    std::int64_t block_height;
    std::uint64_t timestamp;
  };

  void insert_data(const BlockTimestampData &block_data);

  ConstructionToken token;
};

class DefiBlockReserveExport {

public:
  struct ConstructionToken {
    std::string db_host;
    std::string db_user;
    std::string db_pwd;
    std::string db_name;
  };

  DefiBlockReserveExport(const ConstructionToken token) : token(token){};

  void discard();
  void enqueue(std::int64_t block_height, std::uint8_t idA, std::uint8_t idB,
                    std::int64_t reserveA, std::int64_t reserveB);
  void process_queue();

private:
  struct BlockReserveData {
    std::int64_t block_height;
    std::uint8_t idA;
    std::uint8_t idB;
    std::int64_t reserveA;
    std::int64_t reserveB;
  };

  std::vector<BlockReserveData> data_queue_;

  void insert_data(const BlockReserveData &reserve_data);
  std::string create_map_key(const std::uint8_t idA, std::uint8_t idB) const;

  ConstructionToken token;
  AccumulatorMap<BlockAccumulator> acc_map;
};


class DefiPriceExport {

public:
  struct ConstructionToken {
    std::string db_host;
    std::string db_user;
    std::string db_pwd;
    std::string db_name;
  };

  DefiPriceExport(const ConstructionToken token) : token(token){};

  bool export_daily_price(std::int64_t timestamp, std::int64_t idA, std::int64_t idB,
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
