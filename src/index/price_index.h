#ifndef DEFI_INDEX_PRICE_INDEX_H
#define DEFI_INDEX_PRICE_INDEX_H


#include <index/base.h>
#include <index/price_index/price_database.h>
#include <index/price_index/daily_accumulator.h>

#include <memory>
#include <primitives/transaction.h>
#include <primitives/block.h>


namespace price_index {

class PriceIndex final : public BaseIndex
{
public:
    PriceIndex(size_t n_cache_size, bool f_memory, bool f_wipe);
    ~PriceIndex(){};

protected:

    bool WriteBlock(const CBlock& block, const CBlockIndex* pindex) override;
    bool Rewind(const CBlockIndex* current_tip, const CBlockIndex* new_tip) override;

    BaseIndex::DB& GetDB() const override { return *m_db; }
    const char* GetName() const override { return "dexpriceindex"; }

private:
    void init_price_database();

    const std::unique_ptr<BaseIndex::DB> m_db;

    DayAccumulatorMap accumulators;
};


/*
❯ ./src/defi-cli -testnet -rpcuser=test -rpcpassword=123 getrawtransaction 0cea82c948cd8303044ff1ce07d2bfb5102ea98646c4c9629dee1793dc178ba4
040000000001023d1120e05162c11fc2dfd88fa2986753b64ba18dc37aeac116cf0dba87641c230000000017160014a0199189b4042bd39dafdb6f6587c7622a9b7ebafffffffff9cb6459b822d32239ecc7235bb788f105f2549b3de33f83ad3ddfb9636ca7b6010000006a473044022014d847662d361dd5697813cfba552569fd4e5a045878e3c05326cfab4422802902204236f5d19282971bf18dcb90f34c5f19b354e4ccded8bfe148069d7e48cdaeb701210238caaf64df350b482e27b9a2a0389afb1d152611862a497e618f7f1477830fdcffffffff0200000000000000006f6a4c6c446654786c0217a91454f938a1fc131b47152cbcf843e995bcf57fd25987010100000077e70000000000001976a9149b23e6ff864904c9702f96c7569ae33257c2bedd88ac010000000000e1f505000000001976a914e3af83dfd67547498e31ecea3b848a4b756abdb288ac00c89a1d000000000017a91457e2d423518fdbcf89e5a4970fcdd4c92c0cbc3687000247304402202a539f23c6165ada28bac6d95b1f96dbcbebcf78589608f93ef9915db0842ccd022051f9a7d7c1a1ac40d3b948cd29af6b42e6ae8068b641d1435e44260eae16e5a001210204ba78432607eac9752b2113971a8061881447fa13eb76165ac5f50ce185fc020000000000
*/

} // namespace price_index

/// The global dexprice index
extern std::unique_ptr<price_index::PriceIndex> g_dex_price_index;

#endif // DEFI_INDEX_PRICE_INDEX_H