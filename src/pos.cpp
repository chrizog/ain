#include <pos.h>
#include <pos_kernel.h>

#include <chain.h>
#include <chainparams.h>
#include <consensus/merkle.h>
#include <key.h>
#include <logging.h>
#include <masternodes/masternodes.h>
#include <sync.h>
#include <validation.h>

extern RecursiveMutex cs_main;

namespace pos {

bool CheckStakeModifier(const CBlockIndex* pindexPrev, const CBlockHeader& blockHeader) {
    if (blockHeader.hashPrevBlock.IsNull())
        return blockHeader.stakeModifier.IsNull();

    CKeyID key;
    if (!blockHeader.ExtractMinterKey(key)) {
        return error("%s: Can't extract minter key\n", __func__);
    }

    if (pindexPrev->nHeight + 1 >= Params().GetConsensus().GreatWorldHeight) {
        auto nodeId = pcustomcsview->GetMasternodeIdByOperator(key);
        if (!nodeId) {
            return error("%s: No master operator found with minter key\n", __func__);
        }
        auto nodePtr = pcustomcsview->GetMasternode(*nodeId);
        assert(nodePtr);
        key = nodePtr->ownerAuthAddress;
    }

    return blockHeader.stakeModifier == pos::ComputeStakeModifier(pindexPrev->stakeModifier, key);
}

/// Check PoS signatures (PoS block hashes are signed with coinstake out pubkey)
bool CheckHeaderSignature(const CBlockHeader& blockHeader) {
    if (blockHeader.sig.empty()) {
        if (blockHeader.height == 0) {
            return true;
        }
        LogPrintf("CheckBlockSignature: Bad Block - PoS signature is empty\n");
        return false;
    }

    CPubKey recoveredPubKey{};
    if (!recoveredPubKey.RecoverCompact(blockHeader.GetHashToSign(), blockHeader.sig)) {
        LogPrintf("CheckBlockSignature: Bad Block - malformed signature\n");
        return false;
    }

    return true;
}

bool ContextualCheckProofOfStake(const CBlockHeader& blockHeader, const Consensus::Params& params, CheckContextState& ctxState) {
    /// @todo may be this is tooooo optimistic? need more validation?
    if (blockHeader.height == 0 && blockHeader.GetHash() == params.hashGenesisBlock) {
        return true;
    }

    CKeyID minter;
    if (!blockHeader.ExtractMinterKey(minter)) {
        return false;
    }
    uint256 masternodeID;
    int64_t creationHeight;
    std::vector<int64_t> subNodesBlockTime;
    uint16_t timelock{0};
    {
        // check that block minter exists and active at the height of the block
        AssertLockHeld(cs_main);
        auto optMasternodeID = pcustomcsview->GetMasternodeIdByOperator(minter);
        if (!optMasternodeID) {
            return false;
        }
        masternodeID = *optMasternodeID;
        auto nodePtr = pcustomcsview->GetMasternode(masternodeID);
        if (!nodePtr || !nodePtr->IsActive(blockHeader.height, *pcustomcsview)) {
            return false;
        }
        creationHeight = int64_t(nodePtr->creationHeight);

        if (blockHeader.height >= static_cast<uint64_t>(params.EunosPayaHeight)) {
            timelock = pcustomcsview->GetTimelock(masternodeID, *nodePtr, blockHeader.height);
        }

        // Check against EunosPayaHeight here for regtest, does not hurt other networks.
        // Redundant checks, but intentionally kept for easier fork accounting.
        if (blockHeader.height >= static_cast<uint64_t>(params.DakotaCrescentHeight) || blockHeader.height >= static_cast<uint64_t>(params.EunosPayaHeight)) {
            const auto usedHeight = blockHeader.height <= static_cast<uint64_t>(params.EunosHeight) ? creationHeight : blockHeader.height;

            // Get block times
            subNodesBlockTime = pcustomcsview->GetBlockTimes(nodePtr->operatorAuthAddress, usedHeight, creationHeight, timelock);
        }
    }

    // checking PoS kernel is faster, so check it first
    if (!CheckKernelHash(blockHeader.stakeModifier, blockHeader.nBits, creationHeight, blockHeader.GetBlockTime(),blockHeader.height,
                         masternodeID, params, subNodesBlockTime, timelock, ctxState)) {
        return false;
    }

    /// @todo Make sure none mint a big amount of continuous blocks
    return CheckHeaderSignature(blockHeader);
}

bool CheckProofOfStake(const CBlockHeader& blockHeader, const CBlockIndex* pindexPrev, const Consensus::Params& params) {

    // this is our own check of own minted block (just to remember)
    CheckContextState ctxState;
    return CheckStakeModifier(pindexPrev, blockHeader) && ContextualCheckProofOfStake(blockHeader, params, ctxState);
}

unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params::PoS& params, bool newDifficultyAdjust)
{
    if (params.fNoRetargeting)
        return pindexLast->nBits;

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
    const auto& nTargetTimespan = newDifficultyAdjust ? params.nTargetTimespanV2 : params.nTargetTimespan;
    if (nActualTimespan < nTargetTimespan/4)
        nActualTimespan = nTargetTimespan/4;
    if (nActualTimespan > nTargetTimespan*4)
        nActualTimespan = nTargetTimespan*4;

    // Retarget
    const arith_uint256 bnDiffLimit = UintToArith256(params.diffLimit);
    arith_uint256 bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    bnNew *= nActualTimespan;
    bnNew /= nTargetTimespan;

    if (bnNew > bnDiffLimit)
        bnNew = bnDiffLimit;

    return bnNew.GetCompact();
}

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, int64_t blockTime, const Consensus::Params& params)
{
    assert(pindexLast != nullptr);
    if (params.pos.fNoRetargeting)
        return pindexLast->nBits;

    unsigned int nProofOfWorkLimit = UintToArith256(params.pos.diffLimit).GetCompact();

    int nHeight{pindexLast->nHeight + 1};
    bool newDifficultyAdjust{nHeight > params.EunosHeight};

    // Restore previous difficulty adjust on testnet after FC
    if (Params().NetworkIDString() == CBaseChainParams::TESTNET && nHeight >= params.FortCanningHeight) {
        newDifficultyAdjust = false;
    }

    const auto interval = newDifficultyAdjust ? params.pos.DifficultyAdjustmentIntervalV2() : params.pos.DifficultyAdjustmentInterval();
    bool skipChange = newDifficultyAdjust ? (nHeight - params.EunosHeight) % interval != 0 : nHeight % interval != 0;

    // Only change once per difficulty adjustment interval
    if (skipChange)
    {
        // Regtest only
        if (params.pos.fAllowMinDifficultyBlocks)
        {
            // Special difficulty rule for testnet:
            // If the new block's timestamp is more than 2* 30 seconds
            // then allow mining of a min-difficulty block.
            if (blockTime > pindexLast->GetBlockTime() + params.pos.nTargetSpacing*2)
                return nProofOfWorkLimit;
            else
            {
                // Return the last non-special-min-difficulty-rules-block
                const CBlockIndex* pindex = pindexLast;
                while (pindex->pprev && pindex->nHeight % params.pos.DifficultyAdjustmentInterval() != 0 && pindex->nBits == nProofOfWorkLimit)
                    pindex = pindex->pprev;
                return pindex->nBits;
            }
        }
        return pindexLast->nBits;
    }

    // Go back by what we want to be 14 days worth of blocks
    int nHeightFirst = pindexLast->nHeight - (interval - 1);
    assert(nHeightFirst >= 0);
    const CBlockIndex* pindexFirst = pindexLast->GetAncestor(nHeightFirst);
    assert(pindexFirst);

    return pos::CalculateNextWorkRequired(pindexLast, pindexFirst->GetBlockTime(), params.pos, newDifficultyAdjust);
}

std::optional<std::string> SignPosBlock(std::shared_ptr<CBlock> pblock, const CKey &key) {
    // if we are trying to sign a signed proof-of-stake block
    if (!pblock->sig.empty()) {
        throw std::logic_error{"Only non-complete PoS block templates are accepted"};
    }

    bool signingRes = key.SignCompact(pblock->GetHashToSign(), pblock->sig);
    if (!signingRes) {
        return {std::string{} + "Block signing error"};
    }

    return {};
}

std::optional<std::string> CheckSignedBlock(const std::shared_ptr<CBlock>& pblock, const CBlockIndex* pindexPrev, const CChainParams& chainparams) {
    uint256 hashBlock = pblock->GetHash();

    // verify hash target and signature of coinstake tx
    if (!CheckProofOfStake(*(CBlockHeader*)pblock.get(), pindexPrev,  chainparams.GetConsensus()))
        return {std::string{} + "proof-of-stake checking failed"};

    LogPrint(BCLog::STAKING, "new proof-of-stake block found hash: %s\n", hashBlock.GetHex());

    // Found a solution
    if (pblock->hashPrevBlock != pindexPrev->GetBlockHash())
        return {std::string{} + "minted block is stale"};

    return {};
}

}
