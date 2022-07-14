#include "block_state.h"

#include <gtest/gtest.h>

#include <string>
#include <cstdint>
#include <map>

TEST(DefiDbExport, block_state_test) {

  defi_export::BlockState block_state;

  auto is_next_block = block_state.update(0, 100, 200);
  EXPECT_FALSE(is_next_block);

  is_next_block = block_state.update(0, 101, 201);
  EXPECT_FALSE(is_next_block);

  is_next_block = block_state.update(1, 102, 202);
  EXPECT_TRUE(is_next_block);

  auto last_block_1 = block_state.get_last_block();
  EXPECT_EQ(last_block_1.block_height, 0);
  EXPECT_EQ(last_block_1.reserve_a, 101);
  EXPECT_EQ(last_block_1.reserve_b, 201);

  is_next_block = block_state.update(1, 102, 202);
  EXPECT_FALSE(is_next_block);
  is_next_block = block_state.update(1, 103, 203);
  EXPECT_FALSE(is_next_block);
  is_next_block = block_state.update(1, 104, 204);
  EXPECT_FALSE(is_next_block);

  is_next_block = block_state.update(2, 102, 202);
  EXPECT_TRUE(is_next_block);

  auto last_block_2 = block_state.get_last_block();
  EXPECT_EQ(last_block_2.block_height, 1);
  EXPECT_EQ(last_block_2.reserve_a, 104);
  EXPECT_EQ(last_block_2.reserve_b, 204);

  is_next_block = block_state.update(1, 105, 205);
  EXPECT_FALSE(is_next_block);
  is_next_block = block_state.update(2, 106, 206);
  EXPECT_TRUE(is_next_block);

  auto last_block_3 = block_state.get_last_block();
  EXPECT_EQ(last_block_3.block_height, 1);
  EXPECT_EQ(last_block_3.reserve_a, 105);
  EXPECT_EQ(last_block_3.reserve_b, 205);
}

TEST(DefiDbExport, block_state_map) {

  auto build_key = [](const int a, const int b) -> std::string {
    return std::to_string(a) + "-" + std::to_string(b);
  };

  std::map<std::string, defi_export::BlockState> block_states_map;

  auto key_1_2 = build_key(1, 2);
  EXPECT_EQ(key_1_2, "1-2");

  auto& state_1_2_new = block_states_map[key_1_2];
  state_1_2_new.update(1, 100, 200);

  auto& state_1_2_from_map = block_states_map[key_1_2];
  auto is_next_block = state_1_2_from_map.update(2, 101, 201);
  EXPECT_TRUE(is_next_block);
  auto last_block_state = state_1_2_from_map.get_last_block();
  EXPECT_EQ(last_block_state.block_height, 1);
  EXPECT_EQ(last_block_state.reserve_a, 100);
  EXPECT_EQ(last_block_state.reserve_b, 200);

}