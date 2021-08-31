#ifndef DEFI_MASTERNODES_LOAN_H
#define DEFI_MASTERNODES_LOAN_H

#include <amount.h>
#include <uint256.h>

#include <flushablestorage.h>
#include <masternodes/balances.h>
#include <masternodes/res.h>
#include <masternodes/vault.h>
#include <script/script.h>

class CLoanSetCollateralToken
{
public:
    DCT_ID idToken{UINT_MAX};
    CAmount factor;
    uint256 priceFeedTxid;
    uint32_t activateAfterBlock = 0;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(idToken);
        READWRITE(factor);
        READWRITE(priceFeedTxid);
        READWRITE(activateAfterBlock);
    }
};

class CLoanSetCollateralTokenImplementation : public CLoanSetCollateralToken
{
public:
    uint256 creationTx;
    int32_t creationHeight = -1;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITEAS(CLoanSetCollateralToken, *this);
        READWRITE(creationTx);
        READWRITE(creationHeight);
    }
};

struct CLoanSetCollateralTokenMessage : public CLoanSetCollateralToken {
    using CLoanSetCollateralToken::CLoanSetCollateralToken;

    ADD_SERIALIZE_METHODS;
    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITEAS(CLoanSetCollateralToken, *this);
    }
};

class CLoanSetLoanToken
{
public:
    std::string symbol;
    std::string name;
    uint256 priceFeedTxid;
    bool mintable = false;
    CAmount interest = 0;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(symbol);
        READWRITE(name);
        READWRITE(priceFeedTxid);
        READWRITE(mintable);
        READWRITE(interest);
    }
};

class CLoanSetLoanTokenImplementation : public CLoanSetLoanToken
{
public:
    uint256 creationTx;
    int32_t creationHeight = -1;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITEAS(CLoanSetLoanToken, *this);
        READWRITE(creationTx);
        READWRITE(creationHeight);
    }
};

struct CLoanSetLoanTokenMessage : public CLoanSetLoanToken {
    using CLoanSetLoanToken::CLoanSetLoanToken;

    ADD_SERIALIZE_METHODS;
    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITEAS(CLoanSetLoanToken, *this);
    }
};

struct CLoanUpdateLoanTokenMessage : public CLoanSetLoanToken {
    using CLoanSetLoanToken::CLoanSetLoanToken;

    uint256 tokenTx;

    ADD_SERIALIZE_METHODS;
    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITEAS(CLoanSetLoanToken, *this);
        READWRITE(tokenTx);
    }
};

struct CollateralTokenKey
{
    DCT_ID id;
    uint32_t height;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(id);

        if (ser_action.ForRead()) {
            READWRITE(WrapBigEndian(height));
            height = ~height;
        } else {
            uint32_t height_ = ~height;
            READWRITE(WrapBigEndian(height_));
        }
    }
};

struct CLoanSchemeData
{
    uint32_t ratio;
    CAmount rate;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(ratio);
        READWRITE(rate);
    }
};

struct CLoanScheme : public CLoanSchemeData
{
    std::string identifier;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITEAS(CLoanSchemeData,*this);
        READWRITE(identifier);
    }
};

struct CLoanSchemeMessage : public CLoanScheme
{
    uint64_t updateHeight{0};

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITEAS(CLoanScheme,*this);
        READWRITE(updateHeight);
    }
};

struct CDefaultLoanSchemeMessage
{
    std::string identifier;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(identifier);
    }
};

struct CDestroyLoanSchemeMessage : public CDefaultLoanSchemeMessage
{
    uint64_t height{0};

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITEAS(CDefaultLoanSchemeMessage, *this);
        READWRITE(height);
    }
};

struct CInterestRate
{
    uint32_t count = 0;
    uint32_t height = 0;
    CAmount interestToHeight = 0;
    CAmount interestPerBlock = 0;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(count);
        READWRITE(height);
        READWRITE(interestToHeight);
        READWRITE(interestPerBlock);
    }
};

class CLoanView : public virtual CStorageView {
public:
    using CLoanSetCollateralTokenImpl = CLoanSetCollateralTokenImplementation;
    using CLoanSetLoanTokenImpl = CLoanSetLoanTokenImplementation;

    std::unique_ptr<CLoanSetCollateralTokenImpl> GetLoanSetCollateralToken(uint256 const & txid) const;
    Res LoanCreateSetCollateralToken(CLoanSetCollateralTokenImpl const & collToken);
    void ForEachLoanSetCollateralToken(std::function<bool (CollateralTokenKey const &, uint256 const &)> callback, CollateralTokenKey const & start = {DCT_ID{0}, UINT_MAX});
    std::unique_ptr<CLoanSetCollateralTokenImpl> HasLoanSetCollateralToken(CollateralTokenKey const & key);

    std::unique_ptr<CLoanSetLoanTokenImpl> GetLoanSetLoanToken(uint256 const & txid) const;
    std::unique_ptr<CLoanSetLoanTokenImpl> GetLoanSetLoanTokenByID(DCT_ID const & id) const;
    Res LoanSetLoanToken(CLoanSetLoanTokenImpl const & loanToken, DCT_ID const & id);
    Res LoanUpdateLoanToken(CLoanSetLoanTokenImpl const & loanToken, DCT_ID const & id);
    void ForEachLoanSetLoanToken(std::function<bool (DCT_ID const &, CLoanSetLoanTokenImpl const &)> callback, DCT_ID const & start = {0});

    Res StoreLoanScheme(const CLoanSchemeMessage& loanScheme);
    Res StoreDefaultLoanScheme(const std::string& loanSchemeID);
    Res StoreDelayedLoanScheme(const CLoanSchemeMessage& loanScheme);
    Res StoreDelayedDestroyScheme(const CDestroyLoanSchemeMessage& loanScheme);
    Res EraseLoanScheme(const std::string& loanSchemeID);
    void EraseDelayedLoanScheme(const std::string& loanSchemeID, uint64_t height);
    void EraseDelayedDestroyScheme(const std::string& loanSchemeID);
    boost::optional<std::string> GetDefaultLoanScheme();
    boost::optional<CLoanSchemeData> GetLoanScheme(const std::string& loanSchemeID);
    boost::optional<uint64_t> GetDestroyLoanScheme(const std::string& loanSchemeID);
    void ForEachLoanScheme(std::function<bool (const std::string&, const CLoanSchemeData&)> callback);
    void ForEachDelayedLoanScheme(std::function<bool (const std::pair<std::string, uint64_t>&, const CLoanSchemeMessage&)> callback);
    void ForEachDelayedDestroyScheme(std::function<bool (const std::string&, const uint64_t&)> callback);

    boost::optional<CInterestRate> GetInterestRate(const std::string& loanSchemeID, DCT_ID id);
    Res StoreInterest(uint32_t height, const std::string& loanSchemeID, DCT_ID id);
    Res EraseInterest(uint32_t height, const std::string& loanSchemeID, DCT_ID id);

    Res AddLoanToken(const CVaultId& vaultId, CTokenAmount amount);
    Res SubLoanToken(const CVaultId& vaultId, CTokenAmount amount);
    boost::optional<CBalances> GetLoanTokens(const CVaultId& vaultId);
    void ForEachLoanToken(std::function<bool(const CVaultId&, const CBalances&)> callback);

    Res SetLoanLiquidationPenalty(CAmount penalty);
    CAmount GetLoanLiquidationPenalty();

    struct LoanSetCollateralTokenCreationTx { static constexpr uint8_t prefix() { return 0x10; } };
    struct LoanSetCollateralTokenKey        { static constexpr uint8_t prefix() { return 0x11; } };
    struct LoanSetLoanTokenCreationTx       { static constexpr uint8_t prefix() { return 0x12; } };
    struct LoanSetLoanTokenKey              { static constexpr uint8_t prefix() { return 0x13; } };
    struct LoanSchemeKey                    { static constexpr uint8_t prefix() { return 0x14; } };
    struct DefaultLoanSchemeKey             { static constexpr uint8_t prefix() { return 0x15; } };
    struct DelayedLoanSchemeKey             { static constexpr uint8_t prefix() { return 0x16; } };
    struct DestroyLoanSchemeKey             { static constexpr uint8_t prefix() { return 0x17; } };
    struct LoanInterestedRate               { static constexpr uint8_t prefix() { return 0x18; } };
    struct LoanTokenAmount                  { static constexpr uint8_t prefix() { return 0x19; } };
    struct LoanLiquidationPenalty           { static constexpr uint8_t prefix() { return 0x1A; } };
};

#endif // DEFI_MASTERNODES_LOAN_H