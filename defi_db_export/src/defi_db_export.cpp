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

bool DefiPriceExport::export_price(std::int64_t timestamp, std::int64_t idA,
                                   std::int64_t idB, std::int64_t reserveA,
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