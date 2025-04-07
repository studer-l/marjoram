#include "marjoram/maybe.hpp"
#include "marjoram/nothing.hpp"
#include "gtest/gtest.h"
#include <memory>

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
  auto anything = f(true);
  auto nada = f(false);

  EXPECT_EQ(anything.getOrElse(-42), 5);
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
  auto minusOne = std::move(nc).flatMap([](NoCopy_t&&) { return Just(-1); });
  ASSERT_EQ(minusOne.get(), -1);

  ASSERT_EQ(NoCopy_t::newCount, 1LU);

  auto nc2 = Just(NoCopy_t());
  nc2.get().quip();
  ASSERT_EQ(NoCopy_t::newCount, 2LU);

  auto nc3 = Just(NoCopy_t());
  auto maybeNeedy = std::move(nc3).flatMap(
      [](NoCopy_t&& ncc) { return Just(Needy_t(std::move(ncc))); });
  ASSERT_EQ(NoCopy_t::newCount, 3LU);
}

TEST(Maybe, MutableRef) {
  auto nc = Just(NoCopy_t());
  auto minusOne = nc.flatMap([](auto&) { return Just(-1); });
  auto minusOne2 = nc.map([](auto&) { return -1; });
  ASSERT_EQ(minusOne.get(), -1);
  ASSERT_EQ(minusOne2.get(), -1);
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

TEST(Maybe, MoveCtorClearsMovedFrom) {
  ma::Maybe<int> five = Just(5);
  ma::Maybe<int> newFive(std::move(five));

  ASSERT_TRUE(five.isNothing());
  ASSERT_TRUE(newFive.isJust());
}

TEST(Maybe, MoveAssignClearsMovedFrom) {
  ma::Maybe<int> five = Just(5);
  ma::Maybe<int> newFive;
  newFive = std::move(five);

  ASSERT_TRUE(five.isNothing());
  ASSERT_TRUE(newFive.isJust());
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

  auto anything = Complicated(Md, Ei);
  ASSERT_TRUE(anything.isJust());
  ASSERT_EQ(anything.get(), 47);

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
  const DangleTest& ref = Ms.getOrElse(probe);
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

TEST(Maybe, toRight_Happycase) {
  ma::Maybe<int> Ma(42);
  ma::Either<std::string, int> E = Ma.toRight(std::string("wat"));
  ASSERT_TRUE(E.isRight());
  ASSERT_EQ(E.asRight(), 42);
}

TEST(Maybe, toRight_Failcase) {
  ma::Maybe<int> Ma = ma::Nothing;
  auto E = Ma.toRight(std::string("wat"));
  ASSERT_TRUE(E.isLeft());
  ASSERT_EQ(E.asLeft(), std::string("wat"));
}

TEST(Maybe, toLeft_Happycase) {
  ma::Maybe<int> Ma(42);
  ma::Either<int, std::string> E = Ma.toLeft(std::string("wat"));
  ASSERT_TRUE(E.isLeft());
  ASSERT_EQ(E.asLeft(), 42);
}

TEST(Maybe, toLeft_Failcase) {
  ma::Maybe<int> Ma = ma::Nothing;
  auto E = Ma.toLeft(std::string("wat"));
  ASSERT_TRUE(E.isRight());
  ASSERT_EQ(E.asRight(), std::string("wat"));
}

TEST(Maybe, toRight_move) {
  auto nc = Just(NoCopy_t());
  auto e = std::move(nc).toRight(std::string("oops"));
  ASSERT_FALSE(nc.get().hasBrains);
}

TEST(Maybe, toRight_move_default_unused) {
  auto mbstring = Just(std::string("hi"));
  auto e = std::move(mbstring).toRight(NoCopy_t{});
}

TEST(Maybe, toRight_move_default) {
  auto mbstring = ma::Maybe<std::string>{};

  // copy
  auto e = mbstring.toRight(NoCopy_t{});
  ASSERT_TRUE(e.isLeft());
  ASSERT_TRUE(e.asLeft().hasBrains);

  // move
  e = std::move(mbstring).toRight(NoCopy_t{});
  ASSERT_TRUE(e.isLeft());
  ASSERT_TRUE(e.asLeft().hasBrains);
}

TEST(Maybe, toLeft_move) {
  auto nc = Just(NoCopy_t());
  auto e = std::move(nc).toLeft(std::string("oops"));
  ASSERT_FALSE(nc.get().hasBrains);
}

TEST(Maybe, toLeft_move_default_unused) {
  auto mbstring = Just(std::string("hi"));
  auto e = std::move(mbstring).toLeft(NoCopy_t{});
}

TEST(Maybe, toLeft_move_default) {
  auto mbstring = ma::Maybe<std::string>{};

  // copy
  auto e = mbstring.toLeft(NoCopy_t{});
  ASSERT_TRUE(e.isRight());
  ASSERT_TRUE(e.asRight().hasBrains);

  // move
  e = std::move(mbstring).toLeft(NoCopy_t{});
  ASSERT_TRUE(e.isRight());
  ASSERT_TRUE(e.asRight().hasBrains);
}

TEST(Maybe, ToVector) {
  Maybe<float> MbF = ma::Just(42.0f);
  std::vector<float> v(MbF.begin(), MbF.end());
  ASSERT_EQ(v.size(), 1LU);
  ASSERT_EQ(v[0], 42.0f);
}

TEST(Maybe, Assign) {
  ma::Maybe<std::pair<int, int>> idx;
  idx = ma::Just(std::make_pair(2, 4));
}

TEST(Maybe, exists) {
  ma::Maybe<int> justfive = ma::Just(5);
  ASSERT_TRUE(justfive.exists([](int i) { return i > 0; }));
  ASSERT_FALSE(justfive.exists([](int i) { return i % 5; }));

  ma::Maybe<int> notfive = ma::Nothing;
  ASSERT_FALSE(notfive.exists([](int i) { return !(i % 2); }));
  ASSERT_FALSE(notfive.exists([](int i) { return i % 2; }));
}

TEST(Maybe, containsBool) {
  ma::Maybe<bool> justTrue = ma::Just(true);
  ASSERT_TRUE(justTrue.contains(true));
  ASSERT_FALSE(justTrue.contains(false));

  ma::Maybe<bool> nada = ma::Nothing;
  ASSERT_FALSE(nada.contains(true));
  ASSERT_FALSE(nada.contains(false));
}

/*
 *TEST(Maybe, makePair) {
 *  std::pair<float, float> orig(1.0, 2.0);
 *  std::pair<ma::Maybe<float>, int> pOfMb =
 *std::make_pair(ma::Just(orig.first), 2);
 *}
 */

TEST(Maybe, getOrElse_uniquePtr) {
  auto Mbptr = ma::Just(std::make_unique<int>(5));
  ASSERT_NE(Mbptr.getOrElse(nullptr), nullptr);
}
TEST(Maybe, any__of_nothings) {
  ma::Maybe<bool> nada0;
  ma::Maybe<bool> nada1;
  ma::Maybe<bool> nada2;
  const auto& ref = ma::any(nada0, nada1, nada2);
  ASSERT_TRUE(ref.isNothing());
}

TEST(Maybe, any__skips_nothing) {
  ma::Maybe<bool> nada;
  ma::Maybe<bool> justTrue = true;
  auto& ref = ma::any(nada, justTrue);
  ASSERT_TRUE(ref.contains(true));
}

TEST(Maybe, any__ignores_tail) {
  ma::Maybe<bool> nada0;
  ma::Maybe<bool> justFalse = false;
  ma::Maybe<bool> nada1;
  ma::Maybe<bool> justTrue = true;
  auto& ref = ma::any(nada0, justFalse, nada1, justTrue);
  ASSERT_TRUE(ref.contains(false));
}

TEST(Maybe, comparison) {
  ma::Maybe<int> justFive(5);
  ma::Maybe<int> alsoFive(5);
  ma::Maybe<int> notFive(6);
  ma::Maybe<int> nada;

  ASSERT_EQ(justFive, justFive);
  ASSERT_EQ(justFive, alsoFive);

  ASSERT_NE(justFive, nada);
  ASSERT_NE(nada, justFive);
  ASSERT_NE(justFive, notFive);
}

TEST(Maybe, filter) {
  ma::Maybe<int> five(5);
  ma::Maybe<int> four(4);

  auto isEven = [](int i) { return i % 2 == 0; };

  ASSERT_TRUE(five.filter(isEven).isNothing());
  ASSERT_TRUE(four.filter(isEven).contains(4));
}

TEST(Maybe, filter_move) {
  ma::Maybe<std::unique_ptr<int>> uniqueFour(std::make_unique<int>(4));
  auto isEven = [](const std::unique_ptr<int>& i) { return *i % 2 == 0; };
  ASSERT_TRUE(std::move(uniqueFour).filter(isEven).exists([](const auto& ptr) {
    return *ptr == 4;
  }));
}

TEST(Maybe, get_or_else_move) {
  ma::Maybe<std::unique_ptr<int>> uniqueFour(std::make_unique<int>(4));
  auto rlyFour = std::move(uniqueFour).getOrElse(std::make_unique<int>(5));
  ASSERT_EQ(*rlyFour, 4);
}

TEST(Maybe, get_or_else_move_nothing) {
  ma::Maybe<std::unique_ptr<int>> nada = ma::Nothing;
  auto five = std::move(nada).getOrElse(std::make_unique<int>(5));
  ASSERT_EQ(*five, 5);
}

TEST(Maybe, get_or_else_with_lambda) {
  ma::Maybe<std::unique_ptr<int>> nada = ma::Nothing;
  auto five =
      std::move(nada).getOrElseWith([]() { return std::make_unique<int>(5); });
  ASSERT_EQ(*five, 5);
}

std::unique_ptr<int> unique_one() { return std::make_unique<int>(1); }

TEST(Maybe, get_or_else_with_function) {
  ma::Maybe<std::unique_ptr<int>> nada = ma::Nothing;
  auto one = std::move(nada).getOrElseWith(unique_one);
  ASSERT_EQ(*one, 1);
}

TEST(Maybe, equal_nothing) {
  ASSERT_EQ(ma::Nothing, ma::Maybe<int>());
  ASSERT_EQ(ma::Maybe<int>(), ma::Nothing);
  ASSERT_EQ(ma::Maybe<float>(), ma::Maybe<float>());
}

TEST(Maybe, warns_on_discard) {
  auto f = []() { return ma::Maybe<int>{5}; };
  f();  // this line must produce a compiler warning
}

// no move, no copy
struct Pinned {
  Pinned(int val_) : val(val_) {}
  Pinned(Pinned&&) = delete;
  Pinned(const Pinned&) = delete;

  int val;
};

TEST(Maybe, pinned) { ma::Maybe<Pinned> mbPinned{ma::InitInPlace, 4}; }

struct PinnedNoCopyArg {
  PinnedNoCopyArg(std::unique_ptr<int> val_) : val(std::move(val_)) {}
  PinnedNoCopyArg(Pinned&&) = delete;
  PinnedNoCopyArg(const Pinned&) = delete;

  std::unique_ptr<int> val;
};

TEST(Maybe, pinned_perfect_forwarding) {
  ma::Maybe<PinnedNoCopyArg> mbPinned{ma::InitInPlace, nullptr};
}

TEST(Maybe, emplace_pinned) {
  ma::Maybe<Pinned> mbPinned;
  ASSERT_TRUE(mbPinned.isNothing());

  mbPinned.emplace(54);

  ASSERT_TRUE(mbPinned.isJust());
  ASSERT_TRUE(
      mbPinned.exists([](const auto& pinned) { return pinned.val == 54; }));
}

TEST(Maybe, emplace_pinned_no_copy_arg) {
  ma::Maybe<PinnedNoCopyArg> mbPinned;
  ASSERT_TRUE(mbPinned.isNothing());

  mbPinned.emplace(std::make_unique<int>(54));

  ASSERT_TRUE(mbPinned.isJust());
  ASSERT_TRUE(
      mbPinned.exists([](const auto& pinned) { return *pinned.val == 54; }));
}

TEST(Maybe, reset) {
  ma::Maybe<Pinned> mbPinned{ma::InitInPlace, 4};
  ASSERT_TRUE(mbPinned.isJust());
  mbPinned.reset();
  ASSERT_TRUE(mbPinned.isNothing());
}

TEST(Maybe, is_maybe) {
  ma::Maybe<int> a{1};
  ASSERT_TRUE(ma::is_maybe_v<decltype(a)>);
  ASSERT_TRUE(ma::is_maybe_v<decltype(std::move(a))>);
  ASSERT_FALSE(ma::is_maybe_v<int>);
}

TEST(Maybe, mapN) {
  // We first create an object that is only movable, and need if for several
  // reasons:
  //  - To get compiler errors if a move is not handled correctly
  //  - To check if an lvalue reference was moved from
  class Movable {
   public:
    explicit Movable(std::string value_) : value(value_) {}
    Movable(Movable&& other) : value(std::move(other.value)) {}
    Movable& operator=(Movable&& other) {
      value = std::move(other.value);
      return *this;
    }
    Movable(const Movable&& other) = delete;
    Movable& operator=(const Movable& other) = delete;

    size_t size() const { return value.size(); }

   private:
    std::string value;
  };
  // It works with lvalue references
  {
    auto f = [](Movable& msg) { return msg.size(); };
    ma::Maybe<Movable> msg = Movable("abcd");
    auto res = mapN(f, msg);
    ASSERT_TRUE(res.contains(5ul));
    ASSERT_TRUE(msg.isJust());
    ASSERT_EQ(msg.get().size(), 5ul);
  }
  {
    auto f = [](const Movable& msg) { return msg.size(); };
    ma::Maybe<Movable> msg = Movable("abcd");
    auto res = mapN(f, msg);
    ASSERT_TRUE(res.contains(5ul));
    ASSERT_TRUE(msg.isJust());
    ASSERT_EQ(msg.get().size(), 5ul);
  }
  // Also works for const references
  {
    auto f = [](const Movable& msg) { return msg.size(); };
    const ma::Maybe<Movable> msg = Movable("abcd");
    auto res = mapN(f, msg);
    ASSERT_TRUE(res.contains(5ul));
  }
  // It works with rvalue references
  {
    auto f = [](Movable msg) { return msg.size(); };
    ma::Maybe<Movable> msg = Movable("abcd");
    auto res = mapN(f, std::move(msg));
    ASSERT_TRUE(res.contains(5ul));
  }
  {
    auto f = [](const Movable& msg) { return msg.size(); };
    ma::Maybe<Movable> msg = Movable("abcd");
    auto res = mapN(f, std::move(msg));
    ASSERT_TRUE(res.contains(5ul));
  }
  {
    auto f = [](Movable&& msg) { return msg.size(); };
    ma::Maybe<Movable> msg = Movable("abcd");
    auto res = mapN(f, std::move(msg));
    ASSERT_TRUE(res.contains(5ul));
  }
  // It works with mixed everything
  {
    auto f = [](Movable msg1, const Movable& msg2, auto offset, auto joker) {
      return msg1.size() + msg2.size() + offset + joker.size();
    };
    ma::Maybe<Movable> msg1 = Movable("ab");
    ma::Maybe<Movable> msg2 = Movable("abcde");
    auto res = mapN(f, std::move(msg1), msg2, ma::Just(19),
                    ma::Just(std::string{"12"}));
    ASSERT_TRUE(res.contains(30ul));
  }
  // But as soon as there is one nothing
  {
    auto f = [](int i1, int i2) { return i1 + i2; };
    auto res = mapN(f, ma::Just(1), ma::Maybe<int>{});
    // The function does not get called
    ASSERT_TRUE(res.isNothing());
  }
}
