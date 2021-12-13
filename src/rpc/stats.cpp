#include <rpc/stats.h>

#include <rpc/server.h>
#include <rpc/util.h>

bool CRPCStats::add(const std::string& name, const int64_t latency, const int64_t payload)
{
    UniValue data(UniValue::VOBJ),
            latencyObj(UniValue::VOBJ),
            payloadObj(UniValue::VOBJ),
            history(UniValue::VARR);

    int64_t min_latency = latency,
            avg_latency = latency,
            max_latency = latency,
            min_payload = payload,
            avg_payload = payload,
            max_payload = payload,
            count       = 1;

    auto stats = statsRPC.get(name);
    if (!stats.empty()) {
        count = stats["count"].get_int() + 1;

        latencyObj = stats["latency"].get_obj();
        if (latency < latencyObj["min"].get_int()) min_latency = latency;
        if (latency > latencyObj["max"].get_int()) max_latency = latency;;
        avg_latency = latencyObj["avg"].get_int() + (latency - latencyObj["avg"].get_int()) / count;

        payloadObj = stats["payload"].get_obj();
        if (payload < payloadObj["min"].get_int()) min_payload = payload;;
        if (payload > payloadObj["max"].get_int()) max_payload = payload;;
        avg_payload = payloadObj["avg"].get_int() + (payload - payloadObj["avg"].get_int()) / count;

        auto historyArr = stats["history"].get_array();
        auto i = 0;
        if (historyArr.size() == RPC_STATS_HISTORY_SIZE) {
            i++;
        }
        for (; i < historyArr.size(); i++) {
            history.push_back(historyArr[i]);
        }
    }

    data.pushKV("name", name);

    latencyObj.pushKV("min", latency);
    latencyObj.pushKV("avg", latency);
    latencyObj.pushKV("max", latency);
    data.pushKV("latency", latencyObj);

    payloadObj.pushKV("min", payload);
    payloadObj.pushKV("avg", payload);
    payloadObj.pushKV("max", payload);
    data.pushKV("payload", payloadObj);

    data.pushKV("count", count);
    data.pushKV("lastUsedTime", GetSystemTimeInSeconds());

    UniValue historyObj(UniValue::VOBJ);
    historyObj.pushKV("timestamp", GetSystemTimeInSeconds());
    historyObj.pushKV("latency", latency);
    historyObj.pushKV("payload", payload);
    history.push_back(historyObj);
    data.pushKV("history", history);

    return map.pushKV(name, data);
}

static UniValue getrpcstats(const JSONRPCRequest& request)
{
    RPCHelpMan{"getrpcstats",
        "\nList used RPC commands for this session.\n",
        {
            {"command", RPCArg::Type::STR, RPCArg::Optional::NO, "The command to get stats for."},
            {"verbose", RPCArg::Type::BOOL, RPCArg::Optional::OMITTED, "If set, send full history for this command."}
        },
        RPCResults{},
        RPCExamples{
            HelpExampleCli("getrpcstats", "getblockcount") +
            HelpExampleRpc("getrpcstats", "getblockcount")
        },
    }.Check(request);

    auto command = request.params[0].get_str();
    return statsRPC.get(command);
}

static UniValue listrpcstats(const JSONRPCRequest& request)
{
    RPCHelpMan{"listrpcstats",
        "\nList used RPC commands for this session.\n",
        {
            {"verbose", RPCArg::Type::BOOL, RPCArg::Optional::OMITTED, "If set, send full history for each command."}
        },
        RPCResults{},
        RPCExamples{
            HelpExampleCli("listrpcstats", "") +
            HelpExampleRpc("listrpcstats", "")
        },
    }.Check(request);

    UniValue ret(UniValue::VARR);
    for (const auto& key : statsRPC.getKeys())
    {
        ret.push_back(statsRPC.get(key));
    }
    return ret;
}

// clang-format off
static const CRPCCommand commands[] =
{ //  category              name                      actor (function)         argNames
  //  --------------------- ------------------------  -----------------------  ----------
    { "stats",            "getrpcstats",              &getrpcstats,          {"command", "verbose"} },
    { "stats",            "listrpcstats",             &listrpcstats,         {"verbose"} },
};
// clang-format on

void RegisterStatsRPCCommands(CRPCTable &t)
{
    if (gArgs.GetBoolArg("-rpcstats", DEFAULT_RPC_STATS)) {
        for (unsigned int vcidx = 0; vcidx < ARRAYLEN(commands); vcidx++)
            t.appendCommand(commands[vcidx].name, &commands[vcidx]);
    }
}
