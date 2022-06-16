#include "defi_db_export.h"
#include <cstdint>
#include <gtest/gtest.h>

TEST(DefiDbExport, export_price_test) {

  const std::string db_host_port{"tcp://127.0.0.1:3306"};
  const std::string db_user{"root"};
  const std::string db_pwd{"x"};
  const std::string db_name{"temp"};

  defi_export::DefiPriceExport::ConstructionToken token;
  token.db_host = db_host_port;
  token.db_user = db_user;
  token.db_pwd = db_pwd;
  token.db_name = db_name;

  defi_export::DefiPriceExport defi_price_export{token};

  const std::int64_t interval_two_hours = 60 * 60 * 2;
  std::int64_t start_timestamp = 100;

  for (auto i = 0; i < 12 * 10; i++) {
    std::int64_t timestamp = start_timestamp + (i * interval_two_hours);
    defi_price_export.export_price(timestamp, 1, 2, i + 10, i + 20);
    defi_price_export.export_price(timestamp, 1, 2, i + 10, i + 20);
  }

    for (auto i = 0; i < 12 * 10; i++) {
    std::int64_t timestamp = start_timestamp + (i * interval_two_hours);
    defi_price_export.export_price(timestamp, 1, 3, i + 10, i + 20);
    defi_price_export.export_price(timestamp, 1, 3, i + 10, i + 20);
  }

}