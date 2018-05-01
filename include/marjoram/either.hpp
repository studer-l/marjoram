#pragma once

#include <algorithm>
#include <type_traits>

#include "eitherImpl.hpp"
#include "maybe.hpp"
#include "nothing.hpp"

namespace ma {
/**
 * @defgroup Either Either
 * @addtogroup Either
 * @{
 * Either Monad, supporting constants and functions.
 *
 * An instance of `Either<A,B>` contains either an `A` or a `B`. Unlike
 * `std::variant` both `A` and `B` may be the same type.
 *
 * Example
 * -------
 * Given a function that returns either a fully fledged Widget or a descriptive
 * Error message.
 * ~~~
 * class Widget;
 * class BetterWidget;
 * BetterWidget refine(Widget& w);
 *
 * Either<std::string, Widget> requestWidget(double length, int numberOfBells);
 * ~~~
 *
 * Then client code can treat the above return value uniformly:
 *
 * ~~~
 * Either<std::string, Widget> emw = requestWidget(14.2, 3);
 * Either<std::string, BetterWidget> morphed =
 *            emw.map([](Widget& w) { return refine(w); });
 * ~~~
 *
 */

template <typename A, typename B> class EitherIterator;
template <typename A, typename B> class ConstEitherIterator;
template <typename A> class Maybe;

/**
 * Either monad.
 *
 * Disjoint union of two types.
 *
 * Note that a Either is a monad if one of the type arguments are
 * fixed, we chose the left one to remain fixed under flatMap, i.e.,
 * \code
 * Either<A,>B::flatMap(F) -> Either<A, C>
 * \endcode
 * Where C is the result of F(A).
 *
 * Terminology: If an Either<A, B> contains an A value, we say it has a left
 * value, similarly for B and right.
 */
template <typename A, typename B>
class Either : private detail::EitherImpl<A, B> {
 private:
  using impl = detail::EitherImpl<A, B>;
  static_assert(std::is_same<std::decay_t<A>, A>::value,
                "Either<A, B>: A must be a value type.");
  static_assert(std::is_same<std::decay_t<B>, B>::value,
                "Either<A, B>: B must be a value type.");

 public:
  using value_type = B;
  using left_type = A;
  using right_type = B;

  /**
   * Convenience constructor that infers the Either's side (right or left).
   *
   * Disabled if both A and B can be constructed from the input arguments.
   */
  template <typename... Args,
            typename = typename std::enable_if<
                std::is_constructible<A, Args...>::value ^
                std::is_constructible<B, Args...>::value>::type>
  Either(Args&&... args)
      : impl(std::conditional_t<std::is_constructible<A, Args...>::value,
                                LeftSide, RightSide>(),
             std::forward<Args>(args)...) {}

  /**
   * Construct left Either<A, B> containing an A.
   *
   * Note that LeftSide is just a tag, a const static instance is declared as
   * ma::Left for convenience. Example:
   * \code
   * auto e = Either<double, int>(ma::Left, 5);
   * \endcode
   * Constructs a left sided Either containing a double (although a int
   * could have bound to `5` as well).
   */
  template <typename... Args>
  Either(LeftSide /* selects overload */, Args&&... args)
      : impl(Left, std::forward<Args>(args)...) {}

  /**
   * Construct right Either<A, B> containing an B.
   *
   * Note that RightSide is just a tag, a const static instance is declared as
   * ma::Right for convenience.
   */
  template <typename... Args>
  Either(RightSide /* selects overload */, Args&&... args)
      : impl(Right, std::forward<Args>(args)...) {}

  /**
   * Checks whether an `A` is stored.
   * @return true if this `Either<A, B>` contains an A value.
   */
  bool isLeft() const { return impl::side == detail::left; }

  /**
   * Checks whether a `B` is stored.
   * @return true if this `Either<A, B>` contains a `B` value.
   */
  bool isRight() const { return impl::side == detail::right; }

  using impl::asLeft;
  using impl::asRight;

  /**
   * Reduces Either<A, B> to single value via two function objects.
   *
   * @param fa Function object that can be applied to an `A`.
   * @param fb Function object that can be applied to a `B`.
   * @return Result of either `fa(a)` or `fb(b)` depending on which kind of
   * value is contained.
   *
   * Note that both `fa` and `fb` must have the same return type when applied to
   * their respective arguments.
   */
  template <typename Fa, typename Fb>
  auto fold(Fa fa, Fb fb) const -> std::result_of_t<Fa(A)> {
    static_assert(
        std::is_same<std::result_of_t<Fa(A)>, std::result_of_t<Fb(B)>>::value,
        "Either::Fold Fa and Fb must have identical return type.");
    if (impl::side == detail::right) {
      return fb(asRight());
    }
    return fa(asLeft());
  }

  /**
   * Reduces Either<A, B> to single value via two function objects.
   *
   * @param fa Function object that can be applied to an `A`.
   * @param fb Function object that can be applied to a `B`.
   * @return Result of either `fa(a)` or `fb(b)` depending on which kind of
   * value is contained.
   *
   * Note that both `fa` and `fb` must have the same return type when applied to
   * their respective arguments.
   */
  template <typename Fa, typename Fb>
  auto fold(Fa fa, Fb fb) -> std::result_of_t<Fa(A)> {
    static_assert(
        std::is_same<std::result_of_t<Fa(A)>, std::result_of_t<Fb(B)>>::value,
        "Either::Fold Fa and Fb must have identical return type.");

    if (impl::side == detail::right) {
      return fb(asRight());
    }
    return fa(asLeft());
  }

  /**
   * Applies supplied function to stored `B` value if one is available.
   *
   * @param fb Function object. `F::operator()` when called with `const B&`
   * has return type `Either<A, C>`.
   *
   * @return If this object contains a `B` value, the result of `fb(b)` is
   * stored in the returned either. Otherwise, the pre-existing `A` value is
   * copied into the return value.
   */
  template <typename Fb> auto flatMap(Fb fb) const -> std::result_of_t<Fb(B)> {
    using C = typename std::result_of_t<Fb(B)>::right_type;
    if (isRight()) {
      return fb(asRight());
    }
    return Either<A, C>(Left, asLeft());
  }

  /**
   * Applies supplied function to stored `B` value if one is available.
   *
   * @param fb Function object. `F::operator()` when called with `B&&`
   * has return type `Either<A, C>`.
   *
   * @return If this object contains a `B` value, the result of `fb(b)` is
   * stored in the returned either. Otherwise, the pre-existing `A` value is
   * copied into the return value.
   */

  template <typename Fb> auto flatMap(Fb fb) -> std::result_of_t<Fb(B)> {
    using C = typename std::result_of_t<Fb(B)>::right_type;
    if (isRight()) {
      return fb(std::move(asRight()));
    }
    return Either<A, C>(Left, asLeft());
  }

  /**
   * Applies supplied function to stored `B` value if one is available and
   * wraps the result in Either.
   *
   * @param fb Function object. `F::operator()` when called with `B&&` has
   * return type `Either<A, C>`.
   *
   * @return If this object contains a `B` value, the result of `fb(b)` is
   * stored in the returned either. Otherwise, the pre-existing `A` value is
   * copied into the return value.
   */

  template <typename Fb> auto map(Fb fb) -> Either<A, std::result_of_t<Fb(B)>> {
    using C = typename std::result_of_t<Fb(B)>;
    if (isRight()) {
      return Either<A, C>(Right, fb(std::move(asRight())));
    }
    return Either<A, C>(Left, asLeft());
  }

  /**
   * Applies supplied function to stored `B` value if one is available and
   * wraps result in Either.
   *
   * @param fb Function object. `F::operator()` callable with `const B&` and
   * non-void return type.
   *
   * @return If this object contains a `B` value, the result of `fb(b)` is
   * stored in the returned either. Otherwise, the pre-existing `A` value is
   * copied into the return value.
   */
  template <typename Fb>
  auto map(Fb fb) const -> Either<A, std::result_of_t<Fb(B)>> {
    using C = typename std::result_of_t<Fb(B)>;
    if (isRight()) {
      return Either<A, C>(Right, fb(asRight()));
    }
    return Either<A, C>(Left, asLeft());
  }

  /**
   * If right sided either, return result of applying predicate on right.
   * Otherwise return false.
   *
   * @param pred Function object, when called with argument of type `A` must
   * return bool convertible.
   */
  template <class Predicate> bool exists(Predicate pred) const {
    return fold([](const auto&) { return false; }, pred);
  }

  /**
   * Returns contained right value or given default value.
   */
  const B& getOrElse(const B& dflt) {
    if (isRight()) {
      return asRight();
    }
    return dflt;
  }

  /**
   * Swaps left and right, preserving the stored value.
   * @return Copy of this either instance with A and B mirrored.
   */
  Either<B, A> mirror() const {
    if (isRight()) {
      return Either<B, A>(Left, asRight());
    }
    return Either<B, A>(Right, asLeft());
  }

  /**
   * If a `B` is contained, returns `Just(B)`. Otherwise returns `Nothing`.
   * @return `Maybe<B>` containing either the B or nothing.
   */
  Maybe<B> toMaybe() const {
    if (isRight()) {
      return Just(asRight());
    }
    return ma::Nothing;
  }

  /**
   * @return True if contains a `B` value that compares equal to argument `b`.
   */
  template <typename C> bool contains(const C& c) {
    if (isRight()) {
      return asRight() == c;
    }
    return false;
  }

  /**
   * @param Fa Function object. `F::operator()` when called with `A` value has
   * return type `B`.
   * @return Right value if contained, otherwise result of applying `Fa` to `a`.
   */
  template <typename Fa> B recover(Fa fa) const {
    if (isRight()) {
      return asRight();
    }
    return fa(asLeft());
  }

  /**
   * If both left and right have the same type, allows merging to common type.
   * @return Either left or right, depending on which one is set.
   * The purpose of the template argument `Dummy` is to allow SFINAE.
   */
  template <class Dummy = A>
  auto merge() const ->
      typename std::enable_if<std::is_same<A, B>::value, const Dummy&>::type {
    if (isRight()) {
      return asRight();
    }
    return asLeft();
  }

  /**
   * Joins instance of Either through right.
   * Only enabled on objects of the form Either<A, Either<A, C>>.
   * @return Flattened representation Either<A, C>
   *
   * Ignore the dummy template argument, it is used to enable/disable this
   * method.
   */
  template <class BB = B>
  auto rightJoin() const -> typename std::enable_if<
      std::is_same<Either<typename BB::left_type, typename BB::right_type>,
                   right_type>::value &&
          std::is_same<left_type, typename BB::left_type>::value,
      Either<left_type, typename BB::right_type>>::type {
    return flatMap([](const auto& inner) { return inner; });
  }

  /**
   * Joins instance of Either through left.
   * Only enabled on objects of the form Either<Either<C, B>, B>.
   * @return Flattened representation Either<C, B>
   *
   * Ignore the dummy template argument, it is used to enable/disable this
   * method.
   */
  template <class AA = A>
  auto leftJoin() const -> typename std::enable_if<
      std::is_same<Either<typename AA::left_type, typename AA::right_type>,
                   left_type>::value &&
          std::is_same<right_type, typename AA::right_type>::value,
      Either<typename AA::left_type, right_type>>::type {
    // implementation suboptimal: mirror() may introduce temporary copy, but
    // this is pretty elegant
    return mirror()
        .flatMap([](const auto& inner) { return inner.mirror(); })
        .mirror();
  }

  EitherIterator<A, B> begin() { return {*this, true}; }
  ConstEitherIterator<A, B> begin() const { return {*this, true}; }
  ConstEitherIterator<A, B> cbegin() const { return {*this, true}; }

  EitherIterator<A, B> end() { return {*this, false}; }
  ConstEitherIterator<A, B> end() const { return {*this, false}; }
  ConstEitherIterator<A, B> cend() const { return {*this, false}; }
};

/**
 * Right-biased iterator. Allows mutable access to the right value of an Either.
 *
 * Example:
 *
 * @code
 * Either<A, B> e;
 * [...]
 * for (B& b: e) {
 *  // use `b`
 * }
 * @endcode
 * @see ConstEitherIterator
 */
template <typename A, typename B> class EitherIterator {
 public:
  using iterator_category = std::input_iterator_tag;
  using difference_type = size_t;
  using value_type = B;
  using reference = B&;
  using pointer = B*;

  EitherIterator(Either<A, B>& Mb, bool start)
      : Mb_(Mb), start_(start && Mb_.isRight()) {}
  bool operator!=(const EitherIterator<A, B>& other) {
    return start_ != other.start_;
  }

  reference operator*() { return Mb_.asRight(); }

  pointer operator->() { return &Mb_.asRight(); }

  EitherIterator& operator++() {
    start_ = false;
    return *this;
  }

  EitherIterator& operator++(int) {
    EitherIterator ret(Mb_, start_);
    start_ = false;
    return ret;
  }

 private:
  Either<A, B>& Mb_;
  bool start_;
};

/**
 * Right-biased const iterator.
 * @see EitherIterator
 */
template <typename A, typename B> class ConstEitherIterator {
 public:
  using iterator_category = std::input_iterator_tag;
  using difference_type = size_t;
  using value_type = const B;
  using reference = const B&;
  using pointer = const B*;

  ConstEitherIterator(const Either<A, B>& Mb, bool start)
      : Mb_(Mb), start_(start && Mb_.isRight()) {}
  bool operator!=(const ConstEitherIterator<A, B>& other) {
    return start_ != other.start_;
  }

  reference operator*() { return Mb_.asRight(); }

  pointer operator->() { return &Mb_.asRight(); }

  ConstEitherIterator& operator++() {
    start_ = false;
    return *this;
  }

  ConstEitherIterator& operator++(int) {
    ConstEitherIterator ret(Mb_, start_);
    start_ = false;
    return ret;
  }

 private:
  const Either<A, B>& Mb_;
  bool start_;
};
// @}
}  // namespace ma
