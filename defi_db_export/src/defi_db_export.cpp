#include "defi_db_export.h"
#include "date.h"
#include "export_utils.h"
#include "mysql_db.h"

#include "jdbc/cppconn/prepared_statement.h"

#include <chrono>
#include <memory>
#include <sstream>
#include <string>

namespace defi_export {

BlockReserveExport::BlockReserveExport(const ConstructionToken token,
                                       const unsigned int cache_size,
                                       const std::string &table_name)
    : token(token), cache_size_(cache_size), table_name_(table_name) {

  std::stringstream ss_stmt;
  ss_stmt << "CREATE TABLE IF NOT EXISTS ";
  ss_stmt << table_name_;
  ss_stmt << " ( \
    id INT NOT NULL AUTO_INCREMENT PRIMARY KEY, \
	  block_height INT UNSIGNED NOT NULL, \
    id_token_a TINYINT UNSIGNED NOT NULL, \
    id_token_b TINYINT UNSIGNED NOT NULL, \
    reserve_a BIGINT UNSIGNED NOT NULL,\
    reserve_b BIGINT UNSIGNED NOT NULL, \
    UNIQUE(block_height, id_token_a, id_token_b) \
    );";
  stmt_create_block_reserves_table_ = ss_stmt.str();
};

void BlockReserveExport::enqueue(const std::int64_t block_height, const std::uint8_t idA,
                                 const std::uint8_t idB, const std::int64_t reserveA,
                                 const std::int64_t reserveB) {
  BlockReserveData reserve_data;
  reserve_data.block_height = block_height;
  reserve_data.idA = idA;
  reserve_data.idB = idB;
  reserve_data.reserveA = reserveA;
  reserve_data.reserveB = reserveB;
  data_queue_.emplace_back(reserve_data);
}

void BlockReserveExport::discard() { data_queue_.clear(); }

void BlockReserveExport::process_queue() {
  for (auto &sample : data_queue_) {
    const auto map_key = create_map_key(sample.idA, sample.idB);
    auto &acc = block_state_map_[map_key];
    auto should_write_to_disk =
        acc.update(sample.block_height, sample.reserveA, sample.reserveB);

    if (should_write_to_disk) {
      auto last_record = acc.get_last_block();
      BlockReserveData reserve_data;
      reserve_data.block_height = last_record.block_height;
      reserve_data.idA = sample.idA;
      reserve_data.idB = sample.idB;
      reserve_data.reserveA = last_record.reserve_a;
      reserve_data.reserveB = last_record.reserve_b;

      cache_.emplace_back(reserve_data);
      if (cache_.size() >= cache_size_) {
        insert_data(cache_);
        cache_.clear();
      }
    }
  }
}

void BlockReserveExport::remove_blocks(const std::int64_t block_height_start_remove) {
  auto sql_connection = db::connect(token.db_host, token.db_user, token.db_pwd);
  db::execute_statement(*sql_connection, "USE " + token.db_name);
  db::execute_statement(*sql_connection, stmt_create_block_reserves_table_);

  /* Example:
  DELETE FROM block_reserves WHERE block_height>=10;
  */
  const std::string stmt_delete_blocks =
      "DELETE FROM " + table_name_ + " WHERE block_height>=" +
      std::to_string(block_height_start_remove);
  db::execute_statement(*sql_connection, stmt_delete_blocks);
}

void BlockReserveExport::set_cache_size(const unsigned int cache_size) {
  cache_size_ = cache_size;
}

void BlockReserveExport::insert_data(
    const std::vector<BlockReserveData> &block_reserve_data) {

  auto sql_connection = db::connect(token.db_host, token.db_user, token.db_pwd);
  db::execute_statement(*sql_connection, "USE " + token.db_name);
  db::execute_statement(*sql_connection, stmt_create_block_reserves_table_);

  for (auto &sample : block_reserve_data) {
    /* Example:
    INSERT INTO block_reserves (block_height, id_token_a, id_token_b, reserve_a,
    reserve_b) \ VALUES (1, 2, 3, 500, 1000);
    */
    const std::string &prep_stmt_str =
        "INSERT INTO " + table_name_ + " (block_height, id_token_a, id_token_b, reserve_a, reserve_b) \
            VALUES (?, ?, ?, ?, ?);";

    auto prep_stmt = std::unique_ptr<sql::PreparedStatement>(
        sql_connection->prepareStatement(prep_stmt_str));
    prep_stmt->setUInt(1, sample.block_height);
    prep_stmt->setUInt(2, sample.idA);
    prep_stmt->setUInt(3, sample.idB);
    prep_stmt->setUInt64(4, sample.reserveA);
    prep_stmt->setUInt64(5, sample.reserveB);
    prep_stmt->execute();
  }
}

std::string BlockReserveExport::create_map_key(const std::uint8_t idA,
                                               std::uint8_t idB) const {
  std::string key = std::to_string(idA) + "-" + std::to_string(idB);
  return key;
}

} // namespace defi_export