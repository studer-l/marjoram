#include "marjoram/eitherImpl.hpp"
#include "gtest/gtest.h"
#include <memory>
#include <string>

using ma::detail::EitherImpl;
using ma::LeftEither;
using ma::RightEither;

struct LeftOne {
  char c;
  int i;
  float fs[4];
  ~LeftOne() { dtorCount++; }
  static int dtorCount;
};
int LeftOne::dtorCount = 0;

struct RightOne {
  char c;
  ~RightOne() { dtorCount++; }
  static int dtorCount;
};
int RightOne::dtorCount = 0;

TEST(EitherImpl, dtors) {
  /* ensure we start out at zero... */
  LeftOne::dtorCount = 0;
  RightOne::dtorCount = 0;
  { auto ei = EitherImpl<LeftOne, RightOne>(LeftEither); }
  EXPECT_EQ(LeftOne::dtorCount, 1);
  EXPECT_EQ(RightOne::dtorCount, 0);
  { auto ei = EitherImpl<LeftOne, RightOne>(LeftEither); }
  EXPECT_EQ(LeftOne::dtorCount, 2);
  EXPECT_EQ(RightOne::dtorCount, 0);

  { auto ei = EitherImpl<LeftOne, RightOne>(RightEither); }
  EXPECT_EQ(LeftOne::dtorCount, 2);
  EXPECT_EQ(RightOne::dtorCount, 1);
}

TEST(EitherImpl, ctors) {
  auto ei1 = EitherImpl<std::string, int>(LeftEither, "Hoi");
  auto ei2 = EitherImpl<std::string, int>(RightEither, 56);
}

TEST(EitherImpl, uniqueptr) {
  /* test whether EitherImpl compiles with unique ptr member */
  auto ei1 = EitherImpl<std::unique_ptr<int>, int>(RightEither, 5);
  auto ei2 = EitherImpl<int, std::unique_ptr<int>>(LeftEither, 5);
}
