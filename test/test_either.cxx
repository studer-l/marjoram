#include "marjoram/either.hpp"
#include "gtest/gtest.h"
#include <string>

using ma::Either;
using ma::Left;
using ma::Right;

TEST(Either, ctor) {
  auto int_int = Either<int, int>(Left, 5);
  auto float_float = Either<float, float>(Right, 5);
  auto string_double = Either<std::string, double>(4.0);
  auto string_double2 = Either<std::string, double>("Hello");
  auto float_float2 = Either<float, float>(Left);
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
  auto est = std::move(ept).map(
      [](std::unique_ptr<int>&& i) { return SomeType(std::move(i)); });
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
  Either<int, char> theLetterF(Right, 'f');
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
  Either<std::string, std::string> e(Right, question);
  e = e;
  ASSERT_EQ(e.asRight(), question);
  e = e;
  ASSERT_EQ(e.asRight(), question);
}

TEST(Either, uniqueptr) {
  /* test whether Either compiles with unique ptr member */
  auto ei1 = Either<std::unique_ptr<int>, int>(Right, 5);
  auto ei2 = Either<int, std::unique_ptr<int>>(Left, 5);
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
  ma::Either<std::string, int> Mi(Left, "deadbeef");
  ASSERT_TRUE(Mi.isLeft());
  auto len = Mi.recover([](const std::string& str) { return str.length(); });
  ASSERT_EQ(len, 8);
}

TEST(Either, RightRecover) {
  ma::Either<int, std::string> Mi(Right, "deadbeef");
  ASSERT_TRUE(Mi.isRight());
  std::string out = Mi.recover([](int i) { return std::to_string(i); });
  ASSERT_EQ(out, "deadbeef");
}

TEST(Either, Contains) {
  ma::Either<int, int> DefaultFail(Left, 8);
  ASSERT_FALSE(DefaultFail.contains(8));

  ma::Either<int, std::string> Compares(Right, "42");
  ASSERT_FALSE(Compares.contains("some other string"));
  ASSERT_TRUE(Compares.contains("42"));
}

TEST(Either, Merge) {
  ma::Either<std::string, std::string> ss(Left, "test");
  ASSERT_EQ(ss.merge(), "test");

  ma::Either<std::string, std::string> ss2(Right, "test2");
  ASSERT_EQ(ss2.merge(), "test2");

  ma::Either<std::string, int> si(Left, "test");
  // ss3.merge();  // does not compile
}

TEST(Either, exists) {
  ma::Either<int, std::string> Eis(Right, "test");
  ASSERT_TRUE(Eis.exists([](const std::string& s) { return s.length() > 3; }));
  ASSERT_FALSE(Eis.exists([](const std::string& s) { return s.empty(); }));

  ma::Either<int, std::string> Eis2(Left, 5);
  ASSERT_FALSE(
      Eis2.exists([](const std::string& s) { return s.length() > 3; }));
  ASSERT_FALSE(Eis2.exists([](const std::string& s) { return s.empty(); }));
}

TEST(Either, getOrElse) {
  ma::Either<int, std::string> Eis(Right, "test");
  ma::Either<int, std::string> Eis2(Left, 6);

  ASSERT_EQ(Eis.getOrElse("foobar"), "test");
  ASSERT_EQ(Eis2.getOrElse("foobar"), "foobar");
}

TEST(Either, rightJoin_RightRight) {
  ma::Either<int, ma::Either<int, std::string>> Eiis(
      Right, ma::Either<int, std::string>(Right, "hello"));
  auto joined = Eiis.rightJoin();

  ASSERT_TRUE(joined.isRight());
  ASSERT_EQ(joined.asRight(), "hello");
}

TEST(Either, mirror_different_types) {
  Either<std::string, int> Esi(Left, "Test");
  auto Eis = Esi.mirror();
  ASSERT_TRUE(Eis.isRight());
  ASSERT_EQ(Eis.asRight(), "Test");
}

TEST(Either, mirror_both_types_same) {
  Either<int, int> Eii(Right, 5);
  auto Eiim = Eii.mirror();
  ASSERT_TRUE(Eiim.isLeft());
  ASSERT_EQ(Eiim.asLeft(), 5);
}

TEST(Either, rightJoin_RightLeft) {
  ma::Either<int, ma::Either<int, std::string>> Eiis(
      Right, ma::Either<int, std::string>(Left, 42));
  auto joined = Eiis.rightJoin();

  ASSERT_TRUE(joined.isLeft());
  ASSERT_EQ(joined.asLeft(), 42);
}

TEST(Either, rightJoin_Left) {
  ma::Either<int, ma::Either<int, std::string>> Eiis(Left, 5);
  auto joined = Eiis.rightJoin();

  ASSERT_TRUE(joined.isLeft());
  ASSERT_EQ(joined.asLeft(), 5);
}

TEST(Either, leftJoin_Right) {
  ma::Either<ma::Either<std::string, int>, int> Esii(Right, 42);
  auto joined = Esii.leftJoin();

  ASSERT_TRUE(joined.isRight());
  ASSERT_EQ(joined.asRight(), 42);
}

TEST(Either, leftJoin_LeftRight) {
  ma::Either<ma::Either<std::string, int>, int> Esii(
      Left, ma::Either<std::string, int>(Right, 5));
  auto joined = Esii.leftJoin();

  ASSERT_TRUE(joined.isRight());
  ASSERT_EQ(joined.asRight(), 5);
}

TEST(Either, leftJoin_LefttLeft) {
  ma::Either<ma::Either<std::string, int>, int> Esii(
      Left, ma::Either<std::string, int>(Left, "hello"));
  auto joined = Esii.leftJoin();

  ASSERT_TRUE(joined.isLeft());
  ASSERT_EQ(joined.asLeft(), "hello");
}

/** compile time test; Ensure the type deduction inside `toMaybe` works */
TEST(Either, StringToMaybe) {
  ma::Either<int, std::string> Eis("hello world");
  ma::Maybe<std::string> ms = Eis.toMaybe();
}

TEST(Either, operator_equality) {
  ma::Either<int, int> leftFive{ma::Left, 5};
  ma::Either<int, int> rightFive{ma::Right, 5};
  ASSERT_NE(leftFive, rightFive);
  ASSERT_EQ(leftFive.mirror(), rightFive);
  ASSERT_EQ(leftFive, rightFive.mirror());
}

struct Peerless {
  Peerless() {}
  bool operator==(Peerless) = delete;
};

TEST(Either, operator_equality_sfinae) {
  // can declare Either with types that have no operator==
  Either<int, Peerless> erp(ma::Right);
  Either<Peerless, int> elp(ma::Left);

  // fails to compile
  // erp == elp;
}

TEST(Either, filterOrElse_right) {
  ma::Either<std::string, int> rightFour{ma::Right, 4};
  auto isEven = [](int i) { return i % 2 == 0; };
  auto isOdd = [](int i) { return i % 2 == 1; };

  std::string failed("failed");
  ASSERT_TRUE(rightFour.filterOrElse(isEven, failed).contains(4));

  auto filtered = rightFour.filterOrElse(isOdd, failed);
  ASSERT_TRUE(filtered.isLeft());
  ASSERT_EQ(filtered.asLeft(), failed);
}

TEST(Either, filterOrElse_left) {
  ma::Either<std::string, int> leftFive{ma::Left, "hi"};
  auto isEven = [](int i) { return i % 2 == 0; };
  auto filtered = leftFive.filterOrElse(isEven, "failed");
  ASSERT_TRUE(filtered.isLeft());
  ASSERT_EQ(filtered.asLeft(), "hi");
}

TEST(Either, filterOrElse_move_right) {
  ma::Either<std::string, std::unique_ptr<int>> rightFour{
      ma::Right, std::make_unique<int>(4)};
  auto isEven = [](const std::unique_ptr<int>& i) { return *i % 2 == 0; };

  std::string failed("failed");
  ASSERT_TRUE(std::move(rightFour)
                  .filterOrElse(isEven, failed)
                  .exists([](const auto& ptr) { return *ptr == 4; }));
}

TEST(Either, filterOrElse_move_right_fail) {
  ma::Either<std::string, std::unique_ptr<int>> rightFour{
      ma::Right, std::make_unique<int>(4)};
  std::string failed("failed");
  auto isOdd = [](const std::unique_ptr<int>& i) { return *i % 2 == 1; };
  auto filtered = std::move(rightFour).filterOrElse(isOdd, failed);
  ASSERT_TRUE(filtered.isLeft());
  ASSERT_EQ(filtered.asLeft(), failed);
}

TEST(Either, leftMap_const_ref) {
  const ma::Either<int, std::string> leftFour{ma::Left, 4};
  auto msg = leftFour.leftMap([](const int& a) { return std::to_string(a); });
  ma::Either<std::string, std::string> expected{ma::Left, std::string("4")};
  ASSERT_EQ(msg, expected);
}

TEST(Either, leftMap_mutable_ref) {
  ma::Either<int, std::string> leftFour{ma::Left, 4};
  auto eight = leftFour.leftMap([](int& a) { return 2 * a; });
  ma::Either<int, std::string> expected{ma::Left, 8};
  ASSERT_EQ(eight, expected);
}

TEST(Either, leftMap_move_left) {
  ma::Either<std::unique_ptr<int>, std::string> leftFour{
      ma::Left, std::make_unique<int>(4)};
  auto eight = std::move(leftFour).leftMap([](std::unique_ptr<int> a) {
    *a *= 2;
    return a;
  });
  ASSERT_TRUE(eight.isLeft());
  ASSERT_EQ(*eight.asLeft(), 8);
}

TEST(Either, leftFlatMap_const_ref) {
  const ma::Either<int, std::string> leftFour{ma::Left, 4};

  auto eight = leftFour.leftFlatMap([](const int& a) {
    return ma::Either<int, std::string>{ma::Left, 2 * a};
  });
  ma::Either<int, std::string> expected{ma::Left, 8};
  ASSERT_EQ(eight, expected);

  auto msg = leftFour.leftFlatMap([](const int& a) {
    return ma::Either<int, std::string>{ma::Right, std::to_string(a)};
  });
  ma::Either<int, std::string> expected2{ma::Right, "4"};
  ASSERT_EQ(msg, expected2);
}

TEST(Either, leftFlatMap_mutable_ref) {
  ma::Either<int, std::string> leftFour{ma::Left, 4};

  auto eight = leftFour.leftFlatMap([](int& a) {
    return ma::Either<int, std::string>{ma::Left, 2 * a};
  });
  ma::Either<int, std::string> expected{ma::Left, 8};
  ASSERT_EQ(eight, expected);

  auto msg = leftFour.leftFlatMap([](int& a) {
    return ma::Either<int, std::string>{ma::Right, std::to_string(a)};
  });
  ma::Either<int, std::string> expected2{ma::Right, "4"};
  ASSERT_EQ(msg, expected2);
}

TEST(Either, leftFlatMap_move) {
  ma::Either<std::unique_ptr<int>, std::string> leftFour{
      ma::Left, std::make_unique<int>(4)};

  auto eight = std::move(leftFour).leftFlatMap([](std::unique_ptr<int> a) {
    return ma::Either<std::string, std::string>(ma::Left, std::to_string(*a));
  });

  ma::Either<std::string, std::string> expected{ma::Left, "4"};
  ASSERT_EQ(eight, expected);
}
