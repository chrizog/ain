#ifndef DEFI_PRICE_INDEX_PRICE_ENTRY_H
#define DEFI_PRICE_INDEX_PRICE_ENTRY_H

#include <iostream>
#include <sstream>
#include <string>

struct PriceEntry;
inline std::ostream& operator<<(std::ostream& os, const PriceEntry& price_entry);

struct PriceEntry {
    struct Header {
        int64_t timestamp;
        int64_t blockheight;
        std::string tx_id;
    };

    struct PriceData {
        uint32_t id_token_a;
        uint32_t id_token_b;
        int64_t amount_a;
        int64_t amount_b;
    };

    Header header;
    PriceData data;

    std::string ToString()
    {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }
};

inline std::ostream& operator<<(std::ostream& os, const PriceEntry::PriceData& price_data)
{
    os << price_data.id_token_a << ": " << price_data.amount_a << "; " << price_data.id_token_b << ": " << price_data.amount_b;
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const PriceEntry::Header& price_header)
{
    os << price_header.timestamp << "; " << price_header.blockheight << "; " << price_header.tx_id;
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const PriceEntry& price_entry)
{
    os << price_entry.header << std::endl
       << price_entry.data;
    return os;
}


#endif // DEFI_PRICE_INDEX_PRICE_ENTRY_H