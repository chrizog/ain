#include <exception>
#include <index/price_index.h>
#include <index/price_index/add_liquidity_extractor.h>
#include <index/price_index/price_database.h>
#include <index/price_index/price_entry.h>

#include <chain.h>
#include <logging.h>
#include <masternodes/mn_checks.h>

#include <vector>

std::unique_ptr<price_index::PriceIndex> g_dex_price_index;

namespace price_index {

constexpr char DB_BEST_BLOCK = 'B';

PriceIndex::PriceIndex(size_t n_cache_size, bool f_memory, bool f_wipe) // new-line
    : price_storage((GetDataDir() / "dex_prices.db").string()),
      m_db(MakeUnique<BaseIndex::DB>(GetDataDir() / "indexes" / "dexpriceindex", n_cache_size, f_memory, f_wipe))
{
    m_db->Erase(DB_BEST_BLOCK);
    init_price_database();
}

void PriceIndex::init_price_database()
{
    std::vector<price_index::TableColumn> columns;
    columns.push_back(price_index::TableColumn{"timestamp", "INTEGER NOT NULL"});
    columns.push_back(price_index::TableColumn{"blockheight", "INTEGER NOT NULL"});
    columns.push_back(price_index::TableColumn{"tx_id", "TEXT NOT NULL UNIQUE"});
    columns.push_back(price_index::TableColumn{"id_token_a", "INTEGER NOT NULL"});
    columns.push_back(price_index::TableColumn{"id_token_b", "INTEGER NOT NULL"});
    columns.push_back(price_index::TableColumn{"amount_a", "INTEGER NOT NULL"});
    columns.push_back(price_index::TableColumn{"amount_b", "INTEGER NOT NULL"});

    price_index::Table price_table("prices", columns);
    price_storage.execute_transaction(price_table.get_create_statement());
}

bool PriceIndex::WriteBlock(const CBlock& block, const CBlockIndex* pindex)
{
    static uint64_t last_log_timestamp = 0;

    // LogPrintf("PriceIndex WriteBlock\n");
    for (const CTransactionRef& tx_ref : block.vtx) {
        std::vector<unsigned char> metadata;
        CustomTxType tx_type = GuessCustomTxType(*tx_ref, metadata);

        if (CustomTxType::AddPoolLiquidity == tx_type) {
            // LogPrintf("* \nAddPoolLiquidity transaction detected *\n");

            const int64_t timestamp = block.GetBlockTime();
            const int64_t blockheight = pindex->nHeight;
            const std::string tx_id = tx_ref->GetHash().GetHex();

            PriceEntry::Header price_data_header;
            price_data_header.timestamp = timestamp;
            price_data_header.blockheight = blockheight;
            price_data_header.tx_id = tx_id;

            auto price_data = extract_price_data_add_liquidity(*tx_ref);

            PriceEntry price_entry{price_data_header, price_data};

            int64_t current_time = GetTimeMillis();
            if ((current_time - last_log_timestamp) > (20 * 1000))
            {
                LogPrintf("%s\n", price_entry.ToString());
                last_log_timestamp = current_time;
            }
            
            std::vector<price_index::KeyValuePair> key_value_pairs;
            key_value_pairs.push_back(price_index::KeyValuePair{"timestamp", static_cast<int64_t>(price_entry.header.timestamp)});
            key_value_pairs.push_back(price_index::KeyValuePair{"blockheight", static_cast<int64_t>(price_entry.header.blockheight)});
            key_value_pairs.push_back(price_index::KeyValuePair{"tx_id", price_entry.header.tx_id});
            key_value_pairs.push_back(price_index::KeyValuePair{"id_token_a", static_cast<int64_t>(price_entry.data.id_token_a)});
            key_value_pairs.push_back(price_index::KeyValuePair{"id_token_b", static_cast<int64_t>(price_entry.data.id_token_b)});
            key_value_pairs.push_back(price_index::KeyValuePair{"amount_a", static_cast<int64_t>(price_entry.data.amount_a)});
            key_value_pairs.push_back(price_index::KeyValuePair{"amount_b", static_cast<int64_t>(price_entry.data.amount_b)});

            price_index::Insert insert_statement{"prices", key_value_pairs};
            try {
                price_storage.execute_transaction(insert_statement.ToString());
            } catch (std::exception e) {
                LogPrintf("Insert failed at blockheight %d: %s\n", price_entry.header.blockheight, e.what());
            }
        }
    }
    return true;
}

bool PriceIndex::Rewind(const CBlockIndex* current_tip, const CBlockIndex* new_tip)
{
    return BaseIndex::Rewind(current_tip, new_tip);
}

} // namespace price_index
