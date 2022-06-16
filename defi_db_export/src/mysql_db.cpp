#include "mysql_db.h"
#include "jdbc/cppconn/statement.h"

#include <memory>
#include <string>

namespace defi_export {
namespace db {

std::unique_ptr<sql::Connection> connect(const std::string &db_host_port,
                                         const std::string &db_user,
                                         const std::string &db_pwd) {
  sql::mysql::MySQL_Driver *driver;
  driver = sql::mysql::get_mysql_driver_instance();
  return std::move(std::unique_ptr<sql::Connection>(
      driver->connect(db_host_port, db_user, db_pwd)));
}

void execute_statement(sql::Connection &sql_connection,
                       const std::string &sql_statement) {
  std::unique_ptr<sql::Statement> stmt(sql_connection.createStatement());
  stmt->execute(sql_statement);
}

} // namespace db
} // namespace defi_export
