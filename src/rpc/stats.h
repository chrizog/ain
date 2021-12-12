#ifndef DEFI_RPC_STATS_H
#define DEFI_RPC_STATS_H

#include <map>
#include <stdint.h>
#include <univalue.h>
#include <util/time.h>
#include <util/system.h>

#include <iostream>
#include <fstream>

const char * const DEFAULT_STATSFILE = "stats.log";

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

    void save() const {
        fs::path statsPath = GetDataDir() / DEFAULT_STATSFILE;
        fsbridge::ofstream file(statsPath);
        file << map.write() << '\n';
        file.close();
    };

    void load() {
        fs::path statsPath = GetDataDir() / DEFAULT_STATSFILE;
        fsbridge::ifstream file(statsPath);
        if (!file.is_open()) return;

        std::string line;
        file >> line;
        map.read((const std::string)line);
        file.close();
    };
};

extern CRPCStats statsRPC;

#endif // DEFI_RPC_STATS_H
