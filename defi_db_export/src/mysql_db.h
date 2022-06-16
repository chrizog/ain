#ifndef DEFI_EXPORT_MYSQL_DB_H
#define DEFI_EXPORT_MYSQL_DB_H

#include "jdbc/cppconn/connection.h"
#include "jdbc/mysql_driver.h"

#include <memory>
#include <string>

namespace defi_export {
namespace db {

std::unique_ptr<sql::Connection> connect(const std::string &db_host_port,
                                         const std::string &db_user,
                                         const std::string &db_pwd);

void execute_statement(sql::Connection &sql_connection,
                       const std::string &sql_statement);

} // namespace db
} // namespace defi_export

#endif /* DEFI_EXPORT_MYSQL_DB_H */
