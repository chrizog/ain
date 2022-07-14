#include "defi_db_export.h"
#include <cstdint>
#include <gtest/gtest.h>


TEST(DefiDbExport, export_block_reserve_test) {

  const std::string db_host_port{"tcp://127.0.0.1:3306"};
  const std::string db_user{"root"};
  const std::string db_pwd{"x"};
  const std::string db_name{"temp"};
  const std::string test_table{"test_table"};

  defi_export::ConstructionToken token;
  token.db_host = db_host_port;
  token.db_user = db_user;
  token.db_pwd = db_pwd;
  token.db_name = db_name;

  defi_export::BlockReserveExport defi_block_reserve_export{token, 10, test_table};

  std::int64_t start_block_height = 100;

  for (auto i = 0; i < 11; i++) {
    std::int64_t block_height = start_block_height + i;
    defi_block_reserve_export.discard();
    defi_block_reserve_export.enqueue(block_height, 4, 5, i + 10, i + 20);
    defi_block_reserve_export.enqueue(block_height, 4, 5, i + 10, i + 20);
    defi_block_reserve_export.process_queue();
  }

}