#include "marjoram/either.hpp"
#include "gtest/gtest.h"
#include <string>

using ma::Either;
using ma::LeftEither;
using ma::RightEither;

TEST(Either, ctor) {
  auto int_int = Either<int, int>(LeftEither, 5);
  auto float_float = Either<float, float>(RightEither, 5);
  auto string_double = Either<std::string, double>(4.0);
  auto string_double2 = Either<std::string, double>("Hello");
  auto float_float2 = Either<float, float>(LeftEither);
}

TEST(Either, flatMap) {
  auto five = Either<std::string, int>(5);
  auto msqInts = [](int i) { return Either<std::string, int>(i * i); };
  auto squared = five.flatMap(msqInts);
  auto twiceSquared = five.flatMap(msqInts).flatMap(msqInts);

  ASSERT_TRUE(squared.isRight());
  ASSERT_FALSE(squared.isLeft());
  EXPECT_EQ(squared.asRight(), 25);
  ASSERT_TRUE(twiceSquared.isRight());
  EXPECT_EQ(twiceSquared.asRight(), 25 * 25);

  auto notFive = Either<std::string, int>("RA RA RASPUTIN!");
  squared = notFive.flatMap(msqInts);
  twiceSquared = notFive.flatMap(msqInts).flatMap(msqInts);

  ASSERT_FALSE(squared.isRight());
  ASSERT_TRUE(squared.isLeft());
  EXPECT_EQ(squared.asLeft(), "RA RA RASPUTIN!");
  ASSERT_TRUE(twiceSquared.isLeft());
  EXPECT_EQ(twiceSquared.asLeft(), "RA RA RASPUTIN!");
}

TEST(Either, map) {
  auto five = Either<std::string, int>(5);
  auto msqInts = [](int i) { return i * i; };
  auto squared = five.map(msqInts);
  auto twiceSquared = five.map(msqInts).map(msqInts);

  ASSERT_TRUE(squared.isRight());
  ASSERT_FALSE(squared.isLeft());
  EXPECT_EQ(squared.asRight(), 25);
  ASSERT_TRUE(twiceSquared.isRight());
  EXPECT_EQ(twiceSquared.asRight(), 25 * 25);
}

struct SomeType {
  SomeType(std::unique_ptr<int>&& i) : ip(std::move(i)) {}
  std::unique_ptr<int> ip;
};

TEST(Either, unique_ptr) {
  auto ept = Either<std::string, std::unique_ptr<int>>(new int(42));
  auto est =
      ept.map([](std::unique_ptr<int>&& i) { return SomeType(std::move(i)); });
  ASSERT_TRUE(ept.isRight());
  EXPECT_EQ(*est.asRight().ip, 42);
  EXPECT_EQ(ept.asRight(), nullptr);  // after move should be empty -> nullptr
}

TEST(Either, fold) {
  auto eStr = Either<std::string, int>("There was a cat");
  auto eInt = Either<std::string, int>(42);

  auto fa = [](const std::string& s) -> std::size_t { return s.length(); };
  auto fb = [](const int& i) -> std::size_t { return i; };

  auto rStr = eStr.fold(fa, fb);
  auto rInt = eInt.fold(fa, fb);

  EXPECT_EQ(rStr, std::string("There was a cat").length());
  EXPECT_EQ(rInt, 42LU);
}

TEST(Either, For) {
  Either<int, char> theLetterF(RightEither, 'f');
  Either<std::string, char> hello("Hello");

  for (char f : theLetterF) {
    ASSERT_EQ(f, 'f');
  }

  auto b = hello.begin();
  auto e = hello.end();
  ASSERT_FALSE(b != e);

  for (char c : hello) {
    ASSERT_FALSE(c == c);
  }
}

TEST(Either, Assignment) {
  std::string question = "I wonder whether this works";
  Either<std::string, std::string> e(RightEither, question);
  e = e;
  ASSERT_EQ(e.asRight(), question);
  e = e;
  ASSERT_EQ(e.asRight(), question);
}

TEST(Either, uniqueptr) {
  /* test whether Either compiles with unique ptr member */
  auto ei1 = Either<std::unique_ptr<int>, int>(RightEither, 5);
  auto ei2 = Either<int, std::unique_ptr<int>>(LeftEither, 5);
}

TEST(Either, RightCtors) {
  /* copy  assign*/
  Either<std::string, int> Ei(42);
  Either<std::string, int> Ei2(1337);
  Ei2 = Ei;
  ASSERT_EQ(Ei2.asRight(), 42);

  /* copy */
  Either<std::string, int> Ei3{Ei};
  ASSERT_EQ(Ei3.asRight(), 42);

  /* move assign */
  Either<std::string, int> Ei4(89);
  Ei4 = std::move(Ei);
  ASSERT_EQ(Ei3.asRight(), 42);

  /* move */
  Either<std::string, int> Ei5{std::move(Ei2)};
  ASSERT_EQ(Ei3.asRight(), 42);
}

TEST(Either, LeftCtors) {
  /* copy  assign*/
  Either<int, std::string> Ei(42);
  Either<int, std::string> Ei2(1337);
  Ei2 = Ei;
  ASSERT_EQ(Ei2.asLeft(), 42);

  /* copy */
  Either<int, std::string> Ei3{Ei};
  ASSERT_EQ(Ei3.asLeft(), 42);

  /* move assign */
  Either<int, std::string> Ei4(89);
  Ei4 = std::move(Ei);
  ASSERT_EQ(Ei3.asLeft(), 42);

  /* move */
  Either<int, std::string> Ei5{std::move(Ei2)};
  ASSERT_EQ(Ei3.asLeft(), 42);
}

TEST(Either, LeftRecover) {
  ma::Either<std::string, int> Mi(LeftEither, "deadbeef");
  ASSERT_TRUE(Mi.isLeft());
  auto len = Mi.recover([](const std::string& str) { return str.length(); });
  ASSERT_EQ(len, 8);
}

TEST(Either, RightRecover) {
  ma::Either<int, std::string> Mi(RightEither, "deadbeef");
  ASSERT_TRUE(Mi.isRight());
  std::string out = Mi.recover([](int i) { return std::to_string(i); });
  ASSERT_EQ(out, "deadbeef");
}

TEST(Either, Contains) {
  ma::Either<int, int> DefaultFail(LeftEither, 8);
  ASSERT_FALSE(DefaultFail.contains(8));

  ma::Either<int, std::string> Compares(RightEither, "42");
  ASSERT_FALSE(Compares.contains("some other string"));
  ASSERT_TRUE(Compares.contains("42"));
}

TEST(Either, Merge) {
  ma::Either<std::string, std::string> ss(LeftEither, "test");
  ASSERT_EQ(ss.merge(), "test");

  ma::Either<std::string, int> ss2(LeftEither, "test");
  // ss2.merge();  // does not compile
}

TEST(Either, exists) {
  ma::Either<int, std::string> Eis(RightEither, "test");
  ASSERT_TRUE(Eis.exists([](const std::string& s) { return s.length() > 3; }));
  ASSERT_FALSE(Eis.exists([](const std::string& s) { return s.empty(); }));
}

TEST(EIther, getOrElse) {
  ma::Either<int, std::string> Eis(RightEither, "test");
  ma::Either<int, std::string> Eis2(LeftEither, 6);

  ASSERT_EQ(Eis.getOrElse("foobar"), "test");
  ASSERT_EQ(Eis2.getOrElse("foobar"), "foobar");
}