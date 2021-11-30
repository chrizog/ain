#!/usr/bin/env python3
# Copyright (c) 2021 The DeFi Blockchain developers
# Distributed under the MIT software license, see the accompanying
# file LICENSE or http://www.opensource.org/licenses/mit-license.php.

from test_framework.test_framework import DefiTestFramework
from test_framework.authproxy import JSONRPCException
from test_framework.util import (
    assert_equal,
    assert_raises_rpc_error,
)

class TestForcedRewardAddress(DefiTestFramework):
    def set_test_params(self):
        self.num_nodes = 2
        self.setup_clean_chain = True
        self.extra_args = [
            ['-txindex=1', '-txnotokens=0', '-amkheight=50', '-bayfrontheight=50', '-greatworldheight=110'],
            ['-txindex=1', '-txnotokens=0', '-amkheight=50', '-bayfrontheight=50', '-greatworldheight=110'],
        ]

    def skip_test_if_missing_module(self):
        self.skip_if_no_wallet()

    @staticmethod
    def list_unspent_tx(node, address):
        result = []
        vals = node.listunspent()
        for i in range(0, len(vals)):
            if vals[i]['address'] == address:
                result.append(vals[i])
        return result

    @staticmethod
    def unspent_amount(node, address):
        result = 0
        vals = node.listunspent()
        for i in range(0, len(vals)):
            if vals[i]['address'] == address:
                result += vals[i]['amount']
        return result

    def run_test(self):
        self.nodes[0].generate(105)
        self.sync_all([self.nodes[0], self.nodes[1]])

        self.log.info("Create new masternode for test...")
        num_mns = len(self.nodes[0].listmasternodes())
        mn_owner = self.nodes[0].getnewaddress("", "legacy")

        mn_id = self.nodes[0].createmasternode(mn_owner)
        self.nodes[0].generate(1)

        assert_equal(len(self.nodes[0].listmasternodes()), num_mns + 1)
        assert_equal(self.nodes[0].getmasternode(mn_id)[mn_id]['rewardAddress'], '')
        assert_equal(self.nodes[0].getmasternode(mn_id)[mn_id]['ownerAuthAddress'], mn_owner)
        assert_equal(self.nodes[0].getmasternode(mn_id)[mn_id]['operatorAuthAddress'], mn_owner)

        # Test call before for height
        operator_address = self.nodes[0].getnewaddress("", "legacy")
        assert_raises_rpc_error(-32600, "called before GreatWorld height".format(mn_id), self.nodes[0].updatemasternode, mn_id, {'operatorAddress':operator_address})

        self.nodes[0].generate(4)

        # Test call before masternode active
        operator_address = self.nodes[0].getnewaddress("", "legacy")
        assert_raises_rpc_error(-32600, "Masternode {} is not in 'ENABLED' state".format(mn_id), self.nodes[0].updatemasternode, mn_id, {'operatorAddress':operator_address})

        self.nodes[0].generate(5)

        assert_raises_rpc_error(-32600, "Masternode with that operator address already exists", self.nodes[0].updatemasternode, mn_id, {'operatorAddress':mn_owner})

        # node 1 try to update node 0 which should be rejected.
        assert_raises_rpc_error(-5, "Incorrect authorization for {}".format(mn_owner), self.nodes[1].updatemasternode, mn_id, {'operatorAddress':operator_address})

        self.nodes[0].updatemasternode(mn_id, {'operatorAddress':operator_address})
        self.nodes[0].generate(1)
        self.sync_all()

        assert_equal(self.nodes[1].listmasternodes()[mn_id]["operatorAuthAddress"], operator_address)

        # Set forced address
        forced_reward_address = self.nodes[0].getnewaddress("", "legacy")
        assert_raises_rpc_error(-8,
            "The masternode {} does not exist".format("some_bad_mn_id"),
            self.nodes[0].updatemasternode, "some_bad_mn_id", {'rewardAddress':forced_reward_address}
        )
        assert_raises_rpc_error(-8,
            "rewardAddress ({}) does not refer to a P2PKH or P2WPKH address".format("some_bad_address"),
            self.nodes[0].updatemasternode, mn_id, {'rewardAddress':'some_bad_address'}
        )

        self.nodes[0].updatemasternode(mn_id, {'rewardAddress':forced_reward_address})
        self.nodes[0].generate(1)

        assert_equal(self.nodes[0].getmasternode(mn_id)[mn_id]['rewardAddress'], forced_reward_address)
        assert_equal(self.nodes[0].getmasternode(mn_id)[mn_id]['ownerAuthAddress'], mn_owner)
        assert_equal(self.nodes[0].getmasternode(mn_id)[mn_id]['operatorAuthAddress'], operator_address)

        self.nodes[0].updatemasternode(mn_id, {'rewardAddress':''})
        self.nodes[0].generate(1)

        assert_equal(self.nodes[0].getmasternode(mn_id)[mn_id]['rewardAddress'], '')
        assert_equal(self.nodes[0].getmasternode(mn_id)[mn_id]['ownerAuthAddress'], mn_owner)
        assert_equal(self.nodes[0].getmasternode(mn_id)[mn_id]['operatorAuthAddress'], operator_address)

        self.nodes[0].updatemasternode(mn_id, {'rewardAddress':forced_reward_address})
        self.nodes[0].generate(1)

        fra_amount = self.unspent_amount(self.nodes[0], forced_reward_address)
        fra_unspent = self.list_unspent_tx(self.nodes[0], forced_reward_address)
        assert_equal(len(fra_unspent), 0)
        assert_equal(fra_amount, 0)

        self.stop_node(1)
        self.restart_node(0, ['-gen', '-masternode_operator='+operator_address, '-txindex=1', '-txnotokens=0', '-amkheight=50', '-bayfrontheight=50', '-greatworldheight=1'])

        # Mine blocks
        self.nodes[0].generate(300)

        self.nodes[0].updatemasternode(mn_id, {'rewardAddress':''})
        self.nodes[0].generate(1)

        assert(len(self.list_unspent_tx(self.nodes[0], forced_reward_address)) > len(fra_unspent))
        assert(self.unspent_amount(self.nodes[0], forced_reward_address) > fra_amount)

        # CLI Reward address for test -rewardaddress
        cli_reward_address = self.nodes[0].getnewaddress("", "legacy")
        self.log.info(cli_reward_address)

        self.restart_node(0, ['-gen', '-masternode_operator='+operator_address, '-rewardaddress='+cli_reward_address, '-txindex=1', '-txnotokens=0', '-amkheight=50', '-bayfrontheight=50', '-greatworldheight=1'])

        cra_unspent = self.list_unspent_tx(self.nodes[0], cli_reward_address)
        cra_amount = self.unspent_amount(self.nodes[0], cli_reward_address)
        assert_equal(len(cra_unspent), 0)
        assert_equal(cra_amount, 0)

        # Mine blocks
        self.nodes[0].generate(400)

        assert(len(self.list_unspent_tx(self.nodes[0], cli_reward_address)) > len(fra_unspent))
        assert(self.unspent_amount(self.nodes[0], cli_reward_address) > fra_amount)

        # Test updating operator and reward address simultaniously
        new_operator_address = self.nodes[0].getnewaddress("", "legacy")
        new_reward_address = self.nodes[0].getnewaddress("", "legacy")
        self.nodes[0].updatemasternode(mn_id, {'operatorAddress':new_operator_address,'rewardAddress':new_reward_address})
        self.nodes[0].generate(1)

        # Check results
        result = self.nodes[0].getmasternode(mn_id)[mn_id]
        assert_equal(result['rewardAddress'], new_reward_address)
        assert_equal(result['ownerAuthAddress'], mn_owner)
        assert_equal(result['operatorAuthAddress'], new_operator_address)

        # Test empty argument
        try:
            self.nodes[0].updatemasternode(mn_id, {})
        except JSONRPCException as e:
            errorString = e.error['message']
        assert("No update arguments provided" in errorString)

        # Test unknown update type
        unknown_tx = self.nodes[0].updatemasternode(mn_id, {'rewardAddress':new_reward_address})
        unknown_rawtx = self.nodes[0].getrawtransaction(unknown_tx)
        self.nodes[0].clearmempool()

        updated_tx = unknown_rawtx.replace('01020114', '01ff0114')
        self.nodes[0].signrawtransactionwithwallet(updated_tx)

        try:
            self.nodes[0].sendrawtransaction(updated_tx)
        except JSONRPCException as e:
            errorString = e.error['message']
        assert("Unknown update type provided" in errorString)

if __name__ == '__main__':
    TestForcedRewardAddress().main()
