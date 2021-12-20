// Copyright (c) 2017-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef DEFI_CONSENSUS_TX_CHECK_H
#define DEFI_CONSENSUS_TX_CHECK_H

/**
 * Context-independent transaction checking code that can be called outside the
 * DeFi Blockchain server and doesn't depend on chain or mempool state. Transaction
 * verification code that does call server functions or depend on server state
 * belongs in tx_verify.h/cpp instead.
 */

#include <cstdint>
#include <vector>

/// moved here (!!) due to strange linker errors under mac/win builds
extern const std::vector<unsigned char> DfAnchorFinalizeTxMarker;
extern const std::vector<unsigned char> DfAnchorFinalizeTxMarkerPlus;

class CScript;
class CTransaction;
class CValidationState;

bool CheckTransaction(const CTransaction& tx, CValidationState& state, bool fCheckDuplicateInputs=true);

bool ParseScriptByMarker(CScript const & script,
                         const std::vector<unsigned char> & marker,
                         std::vector<unsigned char> & metadata,
                         bool& hasAdditionalOpcodes,
                         bool& hasAdditionalOpcodesGW,
                         uint32_t* customTxExpiration = nullptr,
                         uint8_t* customTxVersion = nullptr);
bool IsAnchorRewardTx(CTransaction const & tx, std::vector<unsigned char> & metadata, bool fortCanning = false, bool greatWorld = false);
bool IsAnchorRewardTxPlus(CTransaction const & tx, std::vector<unsigned char> & metadata, bool fortCanning = false, bool greatWorld = false);

#endif // DEFI_CONSENSUS_TX_CHECK_H
