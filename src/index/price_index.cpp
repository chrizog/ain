
#include <index/price_index.h>
#include <index/price_index/add_liquidity_extractor.h>
#include <index/price_index/price_entry.h>

#include <logging.h>
#include <masternodes/mn_checks.h>

#include <vector>

std::unique_ptr<price_index::PriceIndex> g_dex_price_index;

namespace price_index {

PriceIndex::PriceIndex(size_t n_cache_size, bool f_memory, bool f_wipe)
    : m_db(MakeUnique<BaseIndex::DB>(GetDataDir() / "indexes" / "dexpriceindex", n_cache_size, f_memory, f_wipe))
{
}

bool PriceIndex::WriteBlock(const CBlock& block, const CBlockIndex* pindex)
{
    // LogPrintf("PriceIndex WriteBlock\n");
    for (const CTransactionRef& tx_ref : block.vtx) {
        std::vector<unsigned char> metadata;
        CustomTxType tx_type = GuessCustomTxType(*tx_ref, metadata);

        if (CustomTxType::AddPoolLiquidity == tx_type) {
            LogPrintf("* \nAddPoolLiquidity transaction detected *\n");
            auto price_data = extract_price_data_add_liquidity(*tx_ref);
            PriceEntry price_entry{{}, price_data};
            LogPrintf("%s\n", price_entry.ToString());
        }
    }
    return true;
}

bool PriceIndex::Rewind(const CBlockIndex* current_tip, const CBlockIndex* new_tip)
{
    return BaseIndex::Rewind(current_tip, new_tip);
}

} // namespace price_index
