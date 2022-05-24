#ifndef DEFI_PRICE_INDEX_DATABASE_H
#define DEFI_PRICE_INDEX_DATABASE_H

#include <sstream>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <SQLiteCpp/Database.h>

namespace price_index {


class KeyValuePair
{
public:
    KeyValuePair() = delete;
    explicit KeyValuePair(const std::string key, const double d);
    explicit KeyValuePair(const std::string key, const std::string s);
    explicit KeyValuePair(const std::string key, const int64_t i);
    std::string ValueToString() const;
    std::string Key() const;

private:
    std::string key_;
    std::variant<int64_t, double, std::string> value_;
};

class Insert
{
public:
    Insert() = delete;
    virtual ~Insert(){};
    Insert(const std::string& table, const std::vector<KeyValuePair>& key_value_pairs);
    std::string ToString() const;

private:
    std::string statement;
};

struct TableColumn {
    std::string name;
    std::string attributes;
};

class Delete
{
public:
    Delete() = delete;
    virtual ~Delete(){};
    Delete(const std::string& table, const std::string& condition)
    {
        // DELETE FROM prices WHERE timestamp > 115;
        std::stringstream ss;
        ss << "DELETE FROM " << table << " WHERE " << condition << ";";
        statement_ = ss.str();
    }
    std::string ToString() const { return statement_; };

private:
    std::string statement_;
};

class Table
{
public:
    Table(const std::string name, const std::vector<TableColumn> columns, const std::string constraint = "");
    std::string get_create_statement() const { return create_statement_; }

private:
    const std::string name_;
    const std::vector<TableColumn> columns_;
    std::string create_statement_;
};

class Storage
{
public:
    Storage(const std::string path);
    void execute_transaction(const std::string& statement);

private:
    const std::string path_;
};

} // namespace price_index


#endif // DEFI_PRICE_INDEX_DATABASE_H