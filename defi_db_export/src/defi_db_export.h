#ifndef DEFI_EXPORT_H
#define DEFI_EXPORT_H

#include "block_state.h"

#include <cstdint>
#include <string>
#include <vector>
#include <map>

namespace defi_export {

struct ConstructionToken {
  std::string db_host;
  std::string db_user;
  std::string db_pwd;
  std::string db_name;
};

class BlockReserveExport {

public:
  BlockReserveExport(const ConstructionToken token,
                         const unsigned int cache_size,
                         const std::string& table_name);
  
  ~BlockReserveExport() {};

  void discard();

  void enqueue(const std::int64_t block_height, const std::uint8_t idA, const std::uint8_t idB,
               const std::int64_t reserveA, const std::int64_t reserveB);

  void process_queue();

  void remove_blocks(const std::int64_t block_height_start_remove);

  void set_cache_size(const unsigned int cache_size);

private:
  const std::string table_name_;
  std::string stmt_create_block_reserves_table_;


  struct BlockReserveData {
    std::int64_t block_height;
    std::uint8_t idA;
    std::uint8_t idB;
    std::int64_t reserveA;
    std::int64_t reserveB;
  };

  std::vector<BlockReserveData> data_queue_;

  void insert_data(const std::vector<BlockReserveData> &reserve_data);
  std::string create_map_key(const std::uint8_t idA, std::uint8_t idB) const;

  ConstructionToken token;
  std::map<std::string, BlockState> block_state_map_;

  unsigned int cache_size_;
  std::vector<BlockReserveData> cache_;
};


} // namespace defi_export

#endif /* DEFI_EXPORT_H */
