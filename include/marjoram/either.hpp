#pragma once

#include <algorithm>
#include <type_traits>

#include "eitherImpl.hpp"
#include "maybe.hpp"

namespace marjoram {

template <typename A, typename B> class EitherIterator;
template <typename A, typename B> class ConstEitherIterator;

/**
 * Either monad
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

  /**
   * Set to A
   */
  using left_type = A;
  /**
   * Set to B
   */
  using right_type = B;

  /**
   * Convenience constructor that infers the Either's side (right or left).
   *
   * May be disabled if both A and B can be constructed from the input
   * arguments.
   */
  template <typename... Args,
            typename = typename std::enable_if<
                std::is_constructible<A, Args...>::value ^
                std::is_constructible<B, Args...>::value>::type>
  explicit Either(Args&&... args)
      : impl(std::conditional_t<std::is_constructible<A, Args...>::value,
                                LeftSide, RightSide>(),
             std::forward<Args>(args)...) {}

  /**
   * Construct left Either<A, B> containing an A.
   *
   * Note that LeftSide is just a tag, a const static instance is declared as
   * marjoram::LeftEither for convenience. Example:
   * \code
   * auto e = Either<double, int>(marjoram::LeftEither, 5);
   * \endcode
   * Constructs a left sided Either containing a double (although a int
   * could have bound to `5` as well).
   */
  template <typename... Args>
  explicit Either(LeftSide /* selects overload */, Args&&... args)
      : impl(LeftEither, std::forward<Args>(args)...) {}

  /**
   * Construct right Either<A, B> containing an B.
   *
   * Note that RightSide is just a tag, a const static instance is declared as
   * marjoram::RightEither for convenience.
   */
  template <typename... Args>
  explicit Either(RightSide /* selects overlaod */, Args&&... args)
      : impl(RightEither, std::forward<Args>(args)...) {}

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
    return Either<A, C>(LeftEither, asLeft());
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
    return Either<A, C>(LeftEither, asLeft());
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
      return Either<A, C>(RightEither, fb(std::move(asRight())));
    }
    return Either<A, C>(LeftEither, asLeft());
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
      return Either<A, C>(RightEither, fb(asRight()));
    }
    return Either<A, C>(LeftEither, asLeft());
  }

  /**
   * Swaps left and right, preserving the stored value.
   * @return Copy of this either instance with A and B mirrored.
   */
  Either<B, A> mirror() const {
    if (isRight()) {
      return Either<B, A>(asRight());
    }
    return Either<B, A>(LeftEither, asLeft());
  }

  /**
   * If a `B` is contained, returns `Just(B)`. Otherwise returns `Nothing`.
   * @return `Maybe<B>` containing either the B or nothing.
   */
  Maybe<B> toMaybe() const {
    if (isRight()) {
      return Just(asRight());
    }
    return Nothing;
  }

  EitherIterator<A, B> begin() { return {*this, true}; }
  ConstEitherIterator<A, B> begin() const { return {*this, true}; }
  ConstEitherIterator<A, B> cbegin() const { return {*this, true}; }

  EitherIterator<A, B> end() { return {*this, false}; }
  ConstEitherIterator<A, B> end() const { return {*this, false}; }
  ConstEitherIterator<A, B> cend() const { return {*this, false}; }
};

template <typename A, typename B> class EitherIterator {
 public:
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

template <typename A, typename B> class ConstEitherIterator {
 public:
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
}  // namespace marjoram
