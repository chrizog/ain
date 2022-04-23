#include "add_liquidity_extractor.h"
#include <masternodes/mn_checks.h>
#include <vector>


namespace price_index {

PriceEntry::PriceData extract_price_data_add_liquidity(const CTransaction& add_liq_tx)
{
    /*
    CAccounts from;
    using CAccounts = std::map<CScript, CBalances>;
    typedef std::map<DCT_ID, CAmount> TAmounts;
    typedef int64_t CAmount;
    CTokenAmount has DCT_ID nTokenId and CAmount nValue;
    */
    PriceEntry::PriceData result;

    std::vector<unsigned char> metadata;
    GuessCustomTxType(add_liq_tx, metadata);

    CLiquidityMessage l_message{};
    CDataStream s_metadata(metadata, SER_NETWORK, PROTOCOL_VERSION);
    s_metadata >> l_message;

    // Sum up the balances of the transaction
    std::vector<CTokenAmount> input_token_amounts;

    for (auto& account : l_message.from) {
        TAmounts balances = account.second.balances;

        for (const auto& pair_id_amount : balances) {
            auto current = CTokenAmount{{pair_id_amount.first.v}, pair_id_amount.second};
            input_token_amounts.push_back(current);
        }
        // LogPrintf("**\n");
        // LogPrintf("%s\n", account.first.GetHex());    // CScript
        // LogPrintf("%s\n", account.second.ToString()); // CBalances
    }

    CBalances summed_balance = CBalances::Sum(input_token_amounts);
    // LogPrintf("Sum: %s\n", summed_balance.ToString()); // CBalances


    auto it_balances = summed_balance.balances.begin();
    result.id_token_a = it_balances->first.v;
    result.amount_a = it_balances->second;

    it_balances++;
    result.id_token_b = it_balances->first.v;
    result.amount_b = it_balances->second;

    /*
    if (result.id_token_a > result.id_token_b) {
        auto tmp_token = result.id_token_a;
        auto tmp_amount = result.amount_a;
        result.id_token_a = result.id_token_b;
        result.amount_a = result.amount_a;
        result.id_token_b = tmp_token;
        result.amount_b = tmp_amount;
    }
    */

    // LogPrintf("%d: %d; %d: %d\n", id_token_A, amountA, id_token_B, amountB);
    return result;
}

} // namespace price_index
