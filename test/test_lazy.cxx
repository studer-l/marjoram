#include "marjoram/lazy.hpp"
#include "gtest/gtest.h"
#include <memory>

using ma::Lazy;

struct foo {
  foo()
      : five([this]() {
          evalCount++;
          return 5;
        }),
        a([this]() {
          evalCount++;
          return 'a';
        }),
        evalCount(0) {}

  int get() const { return five.get(); }

  char geta() const { return a.get(); }

  Lazy<int> five;
  const Lazy<char> a;
  int evalCount;
};

TEST(Lazy, get) {
  foo f;

  ASSERT_EQ(f.evalCount, 0);
  ASSERT_EQ(f.get(), 5);
  ASSERT_EQ(f.evalCount, 1);
  ASSERT_EQ(f.get(), 5);
  ASSERT_EQ(f.evalCount, 1);
}

TEST(Lazy, constGet) {
  const foo f;

  ASSERT_EQ(f.evalCount, 0);
  ASSERT_EQ(f.get(), 5);
  ASSERT_EQ(f.evalCount, 1);
  ASSERT_EQ(f.get(), 5);
  ASSERT_EQ(f.evalCount, 1);

  ASSERT_EQ(f.geta(), 'a');
  ASSERT_EQ(f.evalCount, 2);
  ASSERT_EQ(f.geta(), 'a');
  ASSERT_EQ(f.evalCount, 2);
}

TEST(Lazy, map) {
  Lazy<int> five([]() { return 5; });
  Lazy<double> ten = five.map([](const int& i) { return 2.0 * i; });
  ASSERT_EQ(ten.get(), 10.0);
}

TEST(Lazy, mapMove) {
  Lazy<std::unique_ptr<int>> five([]() { return std::make_unique<int>(5); });
  Lazy<double> thief =
      five.map([](std::unique_ptr<int>&& i) { return *i * 2.0; });
  ASSERT_EQ(thief.get(), 10.0);
}

TEST(Lazy, flatMap) {
  Lazy<int> five([]() { return 5; });
  /* Lazy::flatMap is really ugly, wow */
  Lazy<double> ten = five.flatMap(
      [](const int& i) { return Lazy<double>([i]() { return i * 2.0; }); });

  ASSERT_EQ(ten.get(), 10.0);

  Lazy<std::unique_ptr<int>> fivep([]() { return std::make_unique<int>(5); });
  Lazy<double> thief = fivep.flatMap([](std::unique_ptr<int>&& iptr) {
    return Lazy<double>([&iptr]() { return *iptr * 2.0; });
  });
  ASSERT_EQ(thief.get(), 10.0);
}

static int globalEvalCount = 0;

using ma::Flatten;

TEST(Lazy, flatten) {
  Lazy<Lazy<int>> ten([]() {
    globalEvalCount++;
    return Lazy<int>([]() {
      globalEvalCount++;
      return 10;
    });
  });
  Lazy<int> ften = Flatten(ten);
  ASSERT_EQ(globalEvalCount, 0);
  ASSERT_EQ(ften.get(), 10);
  ASSERT_EQ(globalEvalCount, 2);
  ASSERT_EQ(ften.get(), 10);
  ASSERT_EQ(globalEvalCount, 2);
}

TEST(Lazy, For) {
  Lazy<int> five([]() { return 5; });
  ASSERT_FALSE(five.isEvaluated());
  for (auto& i : five) {
    static_assert(std::is_same<decltype(i), const int&>::value, "");
    ASSERT_EQ(i, 5);
  }
  ASSERT_TRUE(five.isEvaluated());

  Lazy<double> ten = five.map([](const int& i) { return 2.0 * i; });
  ASSERT_FALSE(ten.isEvaluated());
  for (auto& d : ten) {
    static_assert(std::is_same<decltype(d), const double&>::value, "");
    ASSERT_EQ(d, 10.0);
  }
  ASSERT_TRUE(ten.isEvaluated());
}
