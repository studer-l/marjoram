#include "marjoram/maybe0.hpp"
#include "gtest/gtest.h"
#include <memory>

using ma::Maybe0;
using ma::Nothing;

TEST(Maybe0, plain_old_pointer) {
  int five = 5;
  auto justFive = ma::Just0(&five);
  auto square = [](int i) { return i * i; };
  auto squared = justFive.map(square);

  ASSERT_TRUE(squared.isJust());
  ASSERT_FALSE(squared.isNothing());
  EXPECT_EQ(squared.get(), 25);
}

struct Lifetime_t {
  Lifetime_t(int v, int& dc) : value(v), dtor_count(dc) {}
  int value;
  int& dtor_count;
  ~Lifetime_t() { dtor_count++; }
};

TEST(Maybe0, shared_ptr) {
  int count = 0;
  {
    auto five = std::make_shared<Lifetime_t>(5, count);
    auto justFive = ma::Just0(std::move(five));
    auto square = [](const Lifetime_t& lt) { return lt.value * lt.value; };
    auto squared = justFive.map(square);

    ASSERT_TRUE(squared.isJust());
    ASSERT_FALSE(squared.isNothing());
    EXPECT_EQ(squared.get(), 25);
    ASSERT_EQ(count, 0);
  }
  ASSERT_EQ(count, 1);
}

TEST(Maybe0, unique_ptr) {
  int count = 0;
  {
    auto five = std::make_unique<Lifetime_t>(5, count);
    auto justFive = ma::Just0(std::move(five));
    auto square = [](const Lifetime_t& lt) { return lt.value * lt.value; };
    auto squared = justFive.map(square);

    ASSERT_TRUE(squared.isJust());
    ASSERT_FALSE(squared.isNothing());
    EXPECT_EQ(squared.get(), 25);
    ASSERT_EQ(count, 0);
  }
  ASSERT_EQ(count, 1);
}
