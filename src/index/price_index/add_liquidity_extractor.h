#ifndef DEFI_PRICE_INDEX_ADD_LIQUIDITY_EXTRACTOR_H
#define DEFI_PRICE_INDEX_ADD_LIQUIDITY_EXTRACTOR_H

#include "price_entry.h"
#include <primitives/transaction.h>

namespace price_index {

PriceEntry::PriceData extract_price_data_add_liquidity(const CTransaction& add_liq_tx);

}


#endif // DEFI_PRICE_INDEX_ADD_LIQUIDITY_EXTRACTOR_H