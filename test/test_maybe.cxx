#include "marjoram/either.hpp"
#include "marjoram/maybe.hpp"
#include "gtest/gtest.h"

using ma::Just;
using ma::Maybe;
using ma::Nothing;

TEST(Maybe, flatMap) {
  auto five = Just(5);
  auto msqInts = [](int i) { return Just(i * i); };
  auto squared = five.flatMap(msqInts);
  auto twiceSquared = five.flatMap(msqInts).flatMap(msqInts);

  ASSERT_TRUE(squared.isJust());
  ASSERT_FALSE(squared.isNothing());
  EXPECT_EQ(squared.get(), 25);
  ASSERT_TRUE(twiceSquared.isJust());
  EXPECT_EQ(twiceSquared.get(), 25 * 25);
}

TEST(Maybe, map) {
  auto five = Just(5);
  auto sqInts = [](const int& i) { return i * i; };
  auto squared = five.map(sqInts);
  auto twiceSquared = five.map(sqInts).map(sqInts);

  ASSERT_TRUE(squared.isJust());
  ASSERT_FALSE(squared.isNothing());
  EXPECT_EQ(squared.get(), 25);
  ASSERT_TRUE(twiceSquared.isJust());
  EXPECT_EQ(twiceSquared.get(), 25 * 25);
}

Maybe<int> f(bool just) {
  if (just) {
    return 5;
  }
  return Nothing;
}

TEST(Maybe, getOrElse) {
  EXPECT_TRUE(f(true).isJust());
  auto something = f(true);
  auto nada = f(false);

  EXPECT_EQ(something.getOrElse(-42), 5);
  EXPECT_EQ(nada.getOrElse(-42), -42);
}

struct NoCopy_t {
  NoCopy_t() {
    hasBrains = true;
    newCount++;
  }
  NoCopy_t(const NoCopy_t&) = delete;
  NoCopy_t(NoCopy_t&& other) {
    other.hasBrains = false;
    hasBrains = true;
    moveCount++;
  }

  void quip() const { return; }

  bool hasBrains;
  static std::size_t moveCount;
  static std::size_t newCount;
};

std::size_t NoCopy_t::moveCount = 0;
std::size_t NoCopy_t::newCount = 0;

struct Needy_t {
  Needy_t(NoCopy_t&& t) {
    EXPECT_TRUE(t.hasBrains) << "Move ctor from invalid object";
  }
};

TEST(Maybe, NoCopyType) {
  auto nc = Just(NoCopy_t());
  auto minusOne = nc.flatMap([](NoCopy_t&&) { return Just(-1); });
  ASSERT_EQ(minusOne.get(), -1);

  ASSERT_EQ(NoCopy_t::newCount, 1LU);

  auto nc2 = Just(NoCopy_t());
  nc2.get().quip();
  ASSERT_EQ(NoCopy_t::newCount, 2LU);

  auto nc3 = Just(NoCopy_t());
  auto maybeNeedy =
      nc3.flatMap([](NoCopy_t&& ncc) { return Just(Needy_t(std::move(ncc))); });
  ASSERT_EQ(NoCopy_t::newCount, 3LU);
}

TEST(Maybe, MoveCtors) {
  /* compile time test */
  auto nc = NoCopy_t();
  ma::Maybe<NoCopy_t> Mnc(std::move(nc));
  ma::Maybe<NoCopy_t> Mnc2 = std::move(Mnc);
}

TEST(Maybe, For) {
  Maybe<int> five = Just(5);
  Maybe<int> notFive;

  for (int i : five) {
    ASSERT_EQ(i, 5);
  }

  auto b = notFive.begin();
  auto e = notFive.end();
  ASSERT_FALSE(b != e);

  for (int i : notFive) {
    ASSERT_FALSE(i == i);
  }
}

TEST(Maybe, ConstFor) {
  const Maybe<int> five = Just(5);
  const Maybe<int> notFive;

  for (int i : five) {
    ASSERT_EQ(i, 5);
  }

  auto b = notFive.begin();
  auto e = notFive.end();
  ASSERT_FALSE(b != e);

  for (int i : notFive) {
    ASSERT_FALSE(i == i);
  }
}

/* arguably this is a compile time test... oh well */
TEST(Maybe, ImmutableFor) {
  const Maybe<int> five = Just(5);

  for (auto& i : five) {
    using nonref_t = std::remove_reference_t<decltype(i)>;
    static_assert(std::is_const<nonref_t>::value,
                  "ConstMaybeIterator iterator must return const references.");
  }
}

TEST(Maybe, MutableFor) {
  Maybe<int> five = Just(5);

  for (auto& i : five) {
    i = 6;
  }
  ASSERT_TRUE(five.isJust());
  ASSERT_EQ(five.get(), 6);
}

TEST(Maybe, NoCopyFor) {
  auto Mnc = Just(NoCopy_t());
  bool ran = false;
  for (auto& nc : Mnc) {
    (void)nc;  // avoid unused variable warning
    ran = true;
  }
  ASSERT_TRUE(ran);
}

using ma::Either;
Maybe<int> Complicated(const Maybe<double>& Md,
                       const Either<std::string, int>& Esi) {
  for (const double& d : Md) {
    for (const int& i : Esi) {
      return Just(static_cast<int>(i + d));
    }
  }
  return Nothing;
}

TEST(Maybe, AdvancedFor) {
  auto Md = Just(5.53);
  auto Mn = Maybe<double>();
  auto Ei = Either<std::string, int>(42);
  auto Es = Either<std::string, int>("Is anybody in there?");

  auto something = Complicated(Md, Ei);
  ASSERT_TRUE(something.isJust());
  ASSERT_EQ(something.get(), 47);

  auto nothing1 = Complicated(Md, Es);
  ASSERT_TRUE(nothing1.isNothing());

  auto nothing2 = Complicated(Mn, Ei);
  ASSERT_TRUE(nothing2.isNothing());
}

struct DangleTest {
  static int dtorCount;
  ~DangleTest() { dtorCount++; }
  DangleTest() = default;
};
int DangleTest::dtorCount = 0;

TEST(Maybe, getOrElseDangle1) {
  Maybe<DangleTest> Ms = Nothing;
  DangleTest probe;
  DangleTest& ref = Ms.getOrElse(probe);
  ASSERT_EQ(&ref, &probe);
  ASSERT_EQ(DangleTest::dtorCount, 0);
}

const DangleTest& wrong_usage() {
  Maybe<DangleTest> Ms = Nothing;
  return Ms.getOrElse(DangleTest());
}

TEST(Maybe, getOrElseDangle2) {
  DangleTest::dtorCount = 0;
  wrong_usage();
  ASSERT_EQ(DangleTest::dtorCount, 1);  // dt dangles
}

TEST(Maybe, CopyCtor) {
  ma::Maybe<std::string> Ms("hi");
  ma::Maybe<std::string> copy = Ms;
}
