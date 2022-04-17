#include "price_database.h"

#include <sstream>
#include <cassert>

#include <SQLiteCpp/Transaction.h>


namespace price_index {

KeyValuePair::KeyValuePair(const std::string key, const double d) : key_(key), value_(d)
{
}
KeyValuePair::KeyValuePair(const std::string key, const std::string s) : key_(key), value_(s)
{
}
KeyValuePair::KeyValuePair(const std::string key, const int64_t i) : key_(key), value_(i)
{
}

std::string KeyValuePair::ValueToString() const
{
    if (std::get_if<std::string>(&value_)) {
        return "'" + std::get<std::string>(value_) + "'";
    } else if (std::get_if<int64_t>(&value_)) {
        return std::to_string(std::get<int64_t>(value_));
    } else if (std::get_if<double>(&value_)) {
        return std::to_string(std::get<double>(value_));
    } else {
        return "";
    };
};

std::string KeyValuePair::Key() const
{
    return key_;
}


Insert::Insert(const std::string& table, const std::vector<KeyValuePair>& key_value_pairs)
{
    assert(key_value_pairs.size() > 0);

    std::stringstream ss;
    ss << "INSERT INTO \"" << table << "\" (";
    for (std::size_t i = 0; i < (key_value_pairs.size() - 1); ++i) {
        ss << "\"" << key_value_pairs[i].Key() << "\", ";
    }
    ss << "\"" << key_value_pairs[key_value_pairs.size() - 1].Key() << "\"";
    ss << ") VALUES (";

    for (std::size_t i = 0; i < (key_value_pairs.size() - 1); ++i) {
        ss << key_value_pairs[i].ValueToString() << ", ";
    }
    ss << key_value_pairs[key_value_pairs.size() - 1].ValueToString() << ");";
    statement = ss.str();
}

std::string Insert::ToString() const
{
    return statement;
}


Table::Table(const std::string name, const std::vector<TableColumn> columns) : name_(name), columns_(columns)
{
    std::stringstream ss;
    ss << "CREATE TABLE IF NOT EXISTS \"" << name << "\" (";
    ss << "\"id\" INTEGER NOT NULL UNIQUE, ";

    for (const auto& column : columns) {
        ss << "\"" << column.name << "\" ";
        ss << column.attributes << ", ";
    }

    ss << "PRIMARY KEY(\"id\" AUTOINCREMENT)";
    ss << ");";
    create_statement_ = ss.str();
};

Storage::Storage(const std::string path) : path_(path),
                                           db_(path_, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE){};


void Storage::execute_transaction(const std::string& statement)
{
    SQLite::Transaction transaction(db_);
    db_.exec(statement);
    transaction.commit();
}


} // namespace price_index
