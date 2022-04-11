#include <boost/test/unit_test.hpp>
#include <test/setup_common.h>

#include <core_io.h>
#include <primitives/transaction.h>

#include <index/price_index/add_liquidity_extractor.h>
#include <index/price_index/price_database.h>

#include <string>
#include <cstdio>
#include <iostream>

// /Library/Developer/CommandLineTools/usr/bin/c++

/*
❯ ./src/defi-cli -testnet -rpcuser=test -rpcpassword=123 getrawtransaction 0cea82c948cd8303044ff1ce07d2bfb5102ea98646c4c9629dee1793dc178ba4
040000000001023d1120e05162c11fc2dfd88fa2986753b64ba18dc37aeac116cf0dba87641c230000000017160014a0199189b4042bd39dafdb6f6587c7622a9b7ebafffffffff9cb6459b822d32239ecc7235bb788f105f2549b3de33f83ad3ddfb9636ca7b6010000006a473044022014d847662d361dd5697813cfba552569fd4e5a045878e3c05326cfab4422802902204236f5d19282971bf18dcb90f34c5f19b354e4ccded8bfe148069d7e48cdaeb701210238caaf64df350b482e27b9a2a0389afb1d152611862a497e618f7f1477830fdcffffffff0200000000000000006f6a4c6c446654786c0217a91454f938a1fc131b47152cbcf843e995bcf57fd25987010100000077e70000000000001976a9149b23e6ff864904c9702f96c7569ae33257c2bedd88ac010000000000e1f505000000001976a914e3af83dfd67547498e31ecea3b848a4b756abdb288ac00c89a1d000000000017a91457e2d423518fdbcf89e5a4970fcdd4c92c0cbc3687000247304402202a539f23c6165ada28bac6d95b1f96dbcbebcf78589608f93ef9915db0842ccd022051f9a7d7c1a1ac40d3b948cd29af6b42e6ae8068b641d1435e44260eae16e5a001210204ba78432607eac9752b2113971a8061881447fa13eb76165ac5f50ce185fc020000000000
*/

BOOST_FIXTURE_TEST_SUITE(price_index_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(DecodeAddPoolLiquidityTest)
{
    std::string raw_tx_in{"040000000001023d1120e05162c11fc2dfd88fa2986753b64ba18dc37aeac116cf0dba87641c230000000017160014a0199189b4042bd39dafdb6f6587c7622a9b7ebafffffffff9cb6459b822d32239ecc7235bb788f105f2549b3de33f83ad3ddfb9636ca7b6010000006a473044022014d847662d361dd5697813cfba552569fd4e5a045878e3c05326cfab4422802902204236f5d19282971bf18dcb90f34c5f19b354e4ccded8bfe148069d7e48cdaeb701210238caaf64df350b482e27b9a2a0389afb1d152611862a497e618f7f1477830fdcffffffff0200000000000000006f6a4c6c446654786c0217a91454f938a1fc131b47152cbcf843e995bcf57fd25987010100000077e70000000000001976a9149b23e6ff864904c9702f96c7569ae33257c2bedd88ac010000000000e1f505000000001976a914e3af83dfd67547498e31ecea3b848a4b756abdb288ac00c89a1d000000000017a91457e2d423518fdbcf89e5a4970fcdd4c92c0cbc3687000247304402202a539f23c6165ada28bac6d95b1f96dbcbebcf78589608f93ef9915db0842ccd022051f9a7d7c1a1ac40d3b948cd29af6b42e6ae8068b641d1435e44260eae16e5a001210204ba78432607eac9752b2113971a8061881447fa13eb76165ac5f50ce185fc020000000000"};
    CMutableTransaction mtx;
    bool try_witness = true;
    bool try_no_witness = true;

    if (!DecodeHexTx(mtx, raw_tx_in, try_no_witness, try_witness)) {
        std::cerr << "Decoding raw transaction failed" << std::endl;
    }

    CTransaction tx(mtx);
    auto price_data = price_index::extract_price_data_add_liquidity(tx);

    // BTC 0.00059255 * 10^7: 59255; ID 1
    // DFI 1.00000000 * 10^7 100000000; ID 0
    std::cerr << "Amount A: " << price_data.amount_a << std::endl;
    std::cerr << "Amount B: " << price_data.amount_b << std::endl;
    std::cerr << "ID Token A: " << price_data.id_token_a << std::endl;
    std::cerr << "ID Token B: " << price_data.id_token_b << std::endl;

    BOOST_CHECK_EQUAL(price_data.amount_a, 100000000);
    BOOST_CHECK_EQUAL(price_data.amount_b, 59255);
    BOOST_CHECK_EQUAL(price_data.id_token_a, 0);
    BOOST_CHECK_EQUAL(price_data.id_token_b, 1);
}

BOOST_AUTO_TEST_CASE(CreateTableTest)
{
    const std::string database_name = "test_database.db";
    auto storage = price_index::Storage(database_name);

    std::vector<price_index::TableColumn> columns;
    columns.push_back(price_index::TableColumn{"timestamp", "INTEGER NOT NULL"});
    columns.push_back(price_index::TableColumn{"blockheight", "INTEGER NOT NULL"});
    columns.push_back(price_index::TableColumn{"tx_id", "TEXT NOT NULL UNIQUE"});
    columns.push_back(price_index::TableColumn{"id_token_a", "INTEGER NOT NULL"});
    columns.push_back(price_index::TableColumn{"id_token_b", "INTEGER NOT NULL"});
    columns.push_back(price_index::TableColumn{"amount_a", "INTEGER NOT NULL"});
    columns.push_back(price_index::TableColumn{"amount_b", "INTEGER NOT NULL"});

    price_index::Table price_table("prices", columns);

    std::cerr << price_table.get_create_statement() << std::endl;   
    storage.execute_transaction(price_table.get_create_statement());

    remove(database_name.c_str());
}

BOOST_AUTO_TEST_CASE(InsertionTest)
{
    const std::string database_name = "test_database_insertion.db";
    auto storage = price_index::Storage(database_name);

    std::vector<price_index::TableColumn> columns;
    columns.push_back(price_index::TableColumn{"timestamp", "INTEGER NOT NULL"});
    columns.push_back(price_index::TableColumn{"tx_id", "TEXT NOT NULL UNIQUE"});
    columns.push_back(price_index::TableColumn{"id_token_a", "INTEGER NOT NULL"});

    price_index::Table price_table("prices", columns);

    std::cerr << price_table.get_create_statement() << std::endl;   
    storage.execute_transaction(price_table.get_create_statement());

    std::vector<price_index::KeyValuePair> key_value_pairs;
    key_value_pairs.push_back(price_index::KeyValuePair{"timestamp", static_cast<int64_t>(100)});
    key_value_pairs.push_back(price_index::KeyValuePair{"tx_id", "123"});
    key_value_pairs.push_back(price_index::KeyValuePair{"id_token_a", static_cast<int64_t>(0)});
    
    price_index::Insert insert_statement{ "prices", key_value_pairs };

    std::cerr << insert_statement.ToString() << std::endl;

    storage.execute_transaction(insert_statement.ToString());

    remove(database_name.c_str());
}

BOOST_AUTO_TEST_SUITE_END()