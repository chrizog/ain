#include "defi_db_export.h"
#include "date.h"
#include "export_utils.h"
#include "mysql_db.h"

#include "jdbc/cppconn/prepared_statement.h"

#include <chrono>
#include <memory>
#include <string>

namespace defi_export {

const std::string stmt_create_prices_table =
    "CREATE TABLE IF NOT EXISTS prices ( \
    id INT NOT NULL AUTO_INCREMENT PRIMARY KEY, \
	  date VARCHAR(64) NOT NULL, \
    id_token_a INT NOT NULL, \
    id_token_b INT NOT NULL, \
    high FLOAT NOT NULL, \
    low FLOAT NOT NULL, \
    UNIQUE(date, id_token_a, id_token_b) \
  );";

const std::string stmt_create_block_reserves_table =
    "CREATE TABLE IF NOT EXISTS block_reserves ( \
    id INT NOT NULL AUTO_INCREMENT PRIMARY KEY, \
	  block_height INT UNSIGNED NOT NULL, \
    id_token_a TINYINT UNSIGNED NOT NULL, \
    id_token_b TINYINT UNSIGNED NOT NULL, \
    reserve_a BIGINT UNSIGNED NOT NULL,\
    reserve_b BIGINT UNSIGNED NOT NULL, \
    UNIQUE(block_height, id_token_a, id_token_b) \
    );";

const std::string stmt_create_timestamp_table =
    "CREATE TABLE IF NOT EXISTS block_timestamps ( \
    id INT NOT NULL AUTO_INCREMENT PRIMARY KEY, \
	  block_height INT UNSIGNED NOT NULL, \
    timestamp BIGINT UNSIGNED NOT NULL,\
    UNIQUE(block_height, timestamp) \
    );";


void DefiBlockTimestampExport::export_block(std::int64_t block_height, std::int64_t timestamp) {
  BlockTimestampData block_data;
  block_data.block_height = block_height;
  block_data.timestamp = timestamp;
  insert_data(block_data);
}

void DefiBlockTimestampExport::remove_blocks(std::int64_t block_height_start_remove) {
  auto sql_connection = db::connect(token.db_host, token.db_user, token.db_pwd);
  db::execute_statement(*sql_connection, "USE " + token.db_name);
  db::execute_statement(*sql_connection, stmt_create_timestamp_table);

  /* Example:
  DELETE FROM block_timestamps WHERE block_height>=10;
  */
  const std::string stmt_delete_blocks = "DELETE FROM block_timestamps WHERE block_height>=" + std::to_string(block_height_start_remove);
  db::execute_statement(*sql_connection, stmt_delete_blocks);
}

void DefiBlockTimestampExport::insert_data(const BlockTimestampData &block_data) {
  auto sql_connection = db::connect(token.db_host, token.db_user, token.db_pwd);
  db::execute_statement(*sql_connection, "USE " + token.db_name);
  db::execute_statement(*sql_connection, stmt_create_timestamp_table);

  /* Example:
  INSERT INTO block_timestamps (block_height, timestamp)  VALUES (1, 2);
  */
  const std::string &prep_stmt_str =
      "INSERT INTO block_timestamps (block_height, timestamp) \
          VALUES (?, ?);";

  auto prep_stmt = std::unique_ptr<sql::PreparedStatement>(
      sql_connection->prepareStatement(prep_stmt_str));
  prep_stmt->setUInt(1, block_data.block_height);
  prep_stmt->setUInt64(2, block_data.timestamp);
  prep_stmt->execute();
}

void DefiBlockReserveExport::enqueue(std::int64_t block_height,
                                     std::uint8_t idA, std::uint8_t idB,
                                     std::int64_t reserveA,
                                     std::int64_t reserveB) {
  BlockReserveData reserve_data;
  reserve_data.block_height = block_height;
  reserve_data.idA = idA;
  reserve_data.idB = idB;
  reserve_data.reserveA = reserveA;
  reserve_data.reserveB = reserveB;
  data_queue_.emplace_back(reserve_data);
}

void DefiBlockReserveExport::discard() { data_queue_.clear(); }

void DefiBlockReserveExport::process_queue() {
  for (auto& sample : data_queue_) {
    auto &acc = acc_map.get_accumulator(create_map_key(sample.idA, sample.idB));
    auto should_write_to_disk = acc.update(sample.block_height, sample.reserveA, sample.reserveB);

    if (should_write_to_disk) {
      auto last_record = acc.get_last_record();
      BlockReserveData reserve_data;
      reserve_data.block_height = sample.block_height;
      reserve_data.idA = sample.idA;
      reserve_data.idB = sample.idB;
      reserve_data.reserveA = last_record.reserveA;
      reserve_data.reserveB = last_record.reserveB;
      insert_data(reserve_data);
    }
  }
}

void DefiBlockReserveExport::insert_data(
    const BlockReserveData &block_reserve_data) {

  auto sql_connection = db::connect(token.db_host, token.db_user, token.db_pwd);
  db::execute_statement(*sql_connection, "USE " + token.db_name);
  db::execute_statement(*sql_connection, stmt_create_block_reserves_table);

  /* Example:
  INSERT INTO block_reserves (block_height, id_token_a, id_token_b, reserve_a,
  reserve_b) \ VALUES (1, 2, 3, 500, 1000);
  */
  const std::string &prep_stmt_str =
      "INSERT INTO block_reserves (block_height, id_token_a, id_token_b, reserve_a, reserve_b) \
          VALUES (?, ?, ?, ?, ?);";

  auto prep_stmt = std::unique_ptr<sql::PreparedStatement>(
      sql_connection->prepareStatement(prep_stmt_str));
  prep_stmt->setUInt(1, block_reserve_data.block_height);
  prep_stmt->setUInt(2, block_reserve_data.idA);
  prep_stmt->setUInt(3, block_reserve_data.idB);
  prep_stmt->setUInt64(4, block_reserve_data.reserveA);
  prep_stmt->setUInt64(5, block_reserve_data.reserveB);
  prep_stmt->execute();
}

std::string DefiBlockReserveExport::create_map_key(const std::uint8_t idA,
                                                   std::uint8_t idB) const {
  std::string key = std::to_string(idA) + "-" + std::to_string(idB);
  return key;
}

bool DefiPriceExport::export_daily_price(std::int64_t timestamp,
                                         std::int64_t idA, std::int64_t idB,
                                         std::int64_t reserveA,
                                         std::int64_t reserveB) {

  auto &acc = acc_map.get_accumulator(create_map_key(idA, idB));

  const double ratio =
      static_cast<double>(reserveB) / static_cast<double>(reserveA);
  auto should_write_to_disk = acc.update(timestamp, ratio);

  if (should_write_to_disk) {
    auto last_record = acc.get_last_record();
    const std::string date_string =
        helper::year_month_day_to_string(last_record.day);

    PriceData price_data;
    price_data.date_str = date_string;
    price_data.idA = idA;
    price_data.idB = idB;
    price_data.high = last_record.high;
    price_data.low = last_record.low;

    insert_data(price_data);
  }
  return true;
}

void DefiPriceExport::insert_data(const PriceData &price_data) {

  auto sql_connection = db::connect(token.db_host, token.db_user, token.db_pwd);
  db::execute_statement(*sql_connection, "USE " + token.db_name);
  db::execute_statement(*sql_connection, stmt_create_prices_table);

  /* Example:
  INSERT INTO prices (date, id_token_a, id_token_b, high, low)
  VALUES ("2022-01-01", 1, 2, 3, 4);
  */
  const std::string &prep_stmt_str =
      "INSERT INTO prices (date, id_token_a, id_token_b, high, low) \
          VALUES (?, ?, ?, ?, ?);";

  auto prep_stmt = std::unique_ptr<sql::PreparedStatement>(
      sql_connection->prepareStatement(prep_stmt_str));
  prep_stmt->setString(1, price_data.date_str);
  prep_stmt->setInt(2, price_data.idA);
  prep_stmt->setInt(3, price_data.idB);
  prep_stmt->setDouble(4, price_data.high);
  prep_stmt->setDouble(5, price_data.low);
  prep_stmt->execute();
}

std::string DefiPriceExport::create_map_key(const std::int64_t idA,
                                            std::int64_t idB) const {
  std::string key = std::to_string(idA) + "-" + std::to_string(idB);
  return key;
}

} // namespace defi_export