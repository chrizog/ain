#ifndef DEFI_RPC_STATS_H
#define DEFI_RPC_STATS_H

#include <map>
#include <stdint.h>
#include <univalue.h>
#include <util/time.h>

/**
 * DeFi Blockchain RPC Stats class.
 */
class CRPCStats
{
private:
    UniValue map{UniValue::VOBJ};
public:
    bool add(const std::string& name, const int64_t latency, const int64_t payload);

    const UniValue get(const std::string& name) { return map[name]; };

    const std::vector<std::string>& getKeys() { return map.getKeys(); };

};

extern CRPCStats statsRPC;

#endif // DEFI_RPC_STATS_H
