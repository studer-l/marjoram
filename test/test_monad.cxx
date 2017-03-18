#include "marjoram/season.hpp"

#include "gtest/gtest.h"

#include <vector>
#include <algorithm>

TEST(Monad, map2) {
  Maybe<std::string> mfizz(std::string("fizz"));
  Maybe<int> mtimes(2);
  auto fizzfizz = map2(mfizz, mtimes, [](const std::string& a, const int& b) {
    std::string ret;
    for (int i = 0; i < b; ++i) {
      ret.append(a);
    }
    return ret;
  });
  ASSERT_TRUE(fizzfizz.isJust());
  ASSERT_EQ(fizzfizz.get(), "fizzfizz");
}

TEST(Monad, map2rvalue) {
  Maybe<std::string> mfizz(std::string("fizz"));
  Maybe<int> mtimes(2);
  auto fizzfizz = map2(Maybe<std::string>("fizz"), Maybe<int>(2),
                       [](const std::string& a, const int& b) {
                         std::string ret;
                         for (int i = 0; i < b; ++i) {
                           ret.append(a);
                         }
                         return ret;
                       });
  ASSERT_TRUE(fizzfizz.isJust());
  ASSERT_EQ(fizzfizz.get(), "fizzfizz");
}

TEST(Monad, sequence) {
  std::vector<Maybe<int>> Mi(10);
  int count = -5;
  std::generate(Mi.begin(), Mi.end(), [&count]() { return Just(count++); });
  auto somes = sequence(Mi);
  ASSERT_TRUE(somes.isJust());

  Mi[4] = Nothing;
  auto notSomes = sequence(Mi);
  ASSERT_TRUE(notSomes.isNothing());
}
