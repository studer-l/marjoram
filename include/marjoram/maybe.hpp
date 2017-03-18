#pragma once

#include <boost/optional/optional.hpp>
#include <type_traits>
#include <utility>

#include "monad.hpp"

namespace marjoram {

/**
 * Convertible to any kind of (empty) `Maybe` object.
 */
struct Nothing_t {
  Nothing_t() {}
};
/**
 * Convenience constant.
 */
static const Nothing_t Nothing;

/* forward declarations */
template <typename A> class Maybe;

template <typename A> Maybe<A> Just(A&& a);

template <typename A, typename... Args> Maybe<A> Just(Args&&... args);

/**
 * Maybe monad.
 *
 * May contain a value or Nothing.
 *
 * Throughout the documentation, if an object of type Maybe<A> contains a
 * value, this value is denoted by `a`.
 */
template <typename A> class Maybe : public Monad<A, Maybe> {
  static_assert(std::is_same<std::decay_t<A>, A>::value,
                "Maybe<A>: A must be a value type.");

 public:
  /**
   * New empty object (containing `Nothing`).
   *
   * Example: `auto empty = Maybe(Nothing);`
   */
  /* implicit */ Maybe(Nothing_t /* overload selection */) : impl_() {}

  /**
   * Construct `Maybe` containing `A` with perfect forwarding.
   *
   * Type requirements:
   * - `A` must be constructible from `args`.
   */
  template <typename... Args>
  explicit Maybe(Args&&... args) : impl_(std::forward<Args>(args)...) {}

  /**
   * Returns result of `f(a)` if this holds a value, otherwise returns Nothing.
   *
   * @param f Function object.
   *
   * Type requirement:
   * - `F::operator()` when called with `const A&` argument  has return type
   *   `Maybe<B>`, where `B` is non void
   *
   * @return `Maybe<B>` containing the result of `f(a)` or `Nothing`.
   */

  template <typename F> auto flatMap(F f) const -> std::result_of_t<F(A)> {
    if (isJust()) {
      return f(get());
    }
    return Nothing;
  }

  /**
   * Returns result of `f(a)` if this holds a value, otherwise returns Nothing.
   *
   * @param f Function object.
   *
   * Type requirement:
   * - `F::operator()` when called with `A&&` argument has return type
   *   `Maybe<B>`, where `B` is non void
   *
   * @return `Maybe<B>` containing the result of `f(a)` or `Nothing`.
   */
  template <typename F> auto flatMap(F f) -> std::result_of_t<F(A)> {
    if (isJust()) {
      return f(std::move(getImpl()));
    }
    return Nothing;
  }

  /**
   * Checks whether this instance contains a value.
   * @return true if this instance contains a value.
   */
  bool isJust() const { return impl_.is_initialized(); }

  /**
   * Checks whether this instance does not contain a value.
   * @return true if this instance does not contain a value.
   */
  bool isNothing() const { return !isJust(); }

  /**
   * Obtains contained value.
   * Undefined behavior if this Maybe does not contain a value.
   * @return const reference to contained value.
   */
  const A& get() const { return getImpl(); }

  /**
   * Obtains contained value.
   * Undefined behavior if this Maybe does not contain a value.
   * @return reference to contained value.
   */
  A& get() { return getImpl(); }

  /**
   * If this object contains a value, returns it. Otherwise returns `dflt`.
   * Undefined behavior if this Maybe does not contain a value.
   * @return reference to contained value.
   */
  A& getOrElse(A& dflt) { return isJust() ? get() : dflt; }

  /**
   * If this object contains a value, returns it. Otherwise returns `dflt`.
   * Undefined behavior if this Maybe does not contain a value.
   * @return reference to contained value.
   */
  const A& getOrElse(const A& dflt) const { return isJust() ? get() : dflt; }

 private:
  // Implementation: boost::optional
  using impl = boost::optional<A>;
  const A& getImpl() const { return impl_.get(); }
  A& getImpl() { return impl_.get(); }

  impl impl_;
};

/**
 * Convenience constructor, copies/moves `a` into new Maybe instance.
 */
template <typename A> Maybe<A> Just(A&& a) {
  return Maybe<A>(std::forward<A>(a));
}

/**
 * Convenience constructor, constructs new Maybe instance containing an `A`
 * using the arguments `args`.
 */
template <typename A, typename... Args> Maybe<A> Just(Args&&... args) {
  return Maybe<A>(std::forward<Args>(args)...);
}
} // namespace marjoram
