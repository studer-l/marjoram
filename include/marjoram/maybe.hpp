#pragma once

#include <boost/optional/optional.hpp>
#include <type_traits>
#include <utility>

namespace ma {
template <typename A> class MaybeIterator;
template <typename A> class ConstMaybeIterator;
/**
 * @defgroup Maybe Maybe
 * @addtogroup Maybe
 * @{
 * Maybe Monad, supporting constants and functions.
 *
 * ~~~
 * template <class T> class Maybe;
 * ~~~
 *
 * Extension to `std::optional`/`boost::optional`, endowing it with `flatMap`.
 *
 * Example
 * -------
 * ~~~
 * class Widget;
 * class BetterWidget;
 * BetterWidget morph(Widget& w);
 *
 * Maybe<Widget> requestWidget(double length, int numberOfBells) {
 *   // do some complicated thing
 *   if (failure) {
 *     return Nothing;
 *   }
 *   return widget;
 * }
 * ~~~
 *
 * The return value of `requestWidget` can be used regardless of whether the
 * operation succeeded by using `map`:
 *
 * ~~~
 * auto mw = requestWidget(14.2, 3);
 * // if a widget was created, morph it
 * Maybe<BetterWidget> morphed = mw.map([](Widget& w) { return morph(w); });
 * ~~~
 *
 * @see Maybe0
 */

/**
 * Convertible to any kind of (empty) Maybe object.
 */
struct Nothing_t {
  Nothing_t() {}
};

/**
 * Convenience constant.
 */
static const Nothing_t Nothing;

/**
 * Maybe monad.
 *
 * May contain a value or Nothing.
 *
 * Throughout the documentation, if an object of type Maybe`<A>` contains a
 * value, this value is denoted by `a`.
 *
 * Type requirements:
 *  A must be a value type
 */
template <typename A> class Maybe {
  static_assert(std::is_same<std::decay_t<A>, A>::value,
                "Maybe<A>: A must be a value type.");

 public:
  using value_type = A;
  /**
   * New empty object (containing `Nothing`).
   */
  /* implicit */ Maybe(Nothing_t /* overload selection */) : impl_() {}

  /**
   * New empty object (containing `Nothing`).
   */
  Maybe() : impl_() {}

  /**
   * Copy `a` into new Maybe instance.
   */
  Maybe(const A& a) : impl_(a) {}

  /**
   * Move `a` into new Maybe instance.
   */
  Maybe(A&& a) : impl_(std::move(a)) {}

  /**
   * Copy Maybe instance.
   */
  Maybe(const Maybe<A>& Ma) = default;

  /**
   * Move Maybe instance.
   */
  Maybe(Maybe<A>&& Ma) = default;

  /**
   * Returns result of `f(a)` wrapped in a Maybe if this holds a value,
   * otherwise returns Nothing.
   *
   * @param f Function object.
   *
   * Type requirement:
   * - `F::operator()` when called with `const A&` argument has return type
   *   `Maybe<B>`, where `B` is non void.
   *
   * @return `Maybe<B>` containing the result of `f(a)` or `Nothing`.
   */

  template <typename F> auto flatMap(F f) const -> std::result_of_t<F(A)> {
    if (isJust()) {
      return f(get());
    }
    return Nothing;  // note that this constraints the return type
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
   * Returns maybe containing result of `f(a)` if this holds a value, otherwise
   * returns Nothing.
   *
   * @param f Function object.
   *
   * Type requirement:
   * - `F::operator()` when called with `const A&` argument has non-void return
   *    type `B`.
   *
   * @return `Maybe<B>` containing the result of `f(a)` or `Nothing`.
   */
  template <typename F> auto map(F f) const -> Maybe<std::result_of_t<F(A)>> {
    if (isJust()) {
      return Maybe<std::result_of_t<F(A)>>((f(get())));
    }
    return Nothing;
  }

  /**
   * Returns maybe containing result of `f(a)` if this holds a value, otherwise
   * returns Nothing.
   *
   * @param f Function object.
   *
   * Type requirement:
   * - `F::operator()` when called with argument of type `A&&` has non-void
   * return type `B`.
   *
   * @return `Maybe<B>` containing the result of `f(a)` or `Nothing`.
   */
  template <typename F> auto map(F f) -> Maybe<std::result_of_t<F(A)>> {
    if (isJust()) {
      return Maybe<std::result_of_t<F(A)>>((f(std::move(get()))));
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
   * @return reference to contained value.
   */
  A& getOrElse(A& dflt) { return isJust() ? get() : dflt; }

  /**
   * If this object contains a value, returns it. Otherwise returns `dflt`.
   * @return reference to contained value.
   */
  const A& getOrElse(const A& dflt) const { return isJust() ? get() : dflt; }

  MaybeIterator<A> begin() { return {*this, true}; }
  ConstMaybeIterator<A> begin() const { return {*this, true}; }
  ConstMaybeIterator<A> cbegin() const { return {*this, true}; }

  MaybeIterator<A> end() { return {*this, false}; }
  ConstMaybeIterator<A> end() const { return {*this, false}; }
  ConstMaybeIterator<A> cend() const { return {*this, false}; }

 private:
  const A& getImpl() const { return impl_.get(); }
  A& getImpl() { return impl_.get(); }

  boost::optional<A> impl_;
};

/**
 * Iterator over Maybe. Allows mutable access to value contained.
 * If the Maybe instance from which the iterator was created contains a value,
 * the loop body is executed.
 *
 * Example:
 *
 * @code
 * Maybe<A> Ma;
 * [...]
 * for (A& a: Ma) {
 *  // use `a`
 * }
 * @endcode
 * @see ConstMaybeIterator
 */
template <typename A> class MaybeIterator {
 public:
  using value_type = A;
  using reference = A&;
  using pointer = A*;

  MaybeIterator(Maybe<A>& Ma, bool start)
      : Ma_(Ma), start_(start && Ma.isJust()) {}
  bool operator!=(const MaybeIterator<A>& other) {
    return start_ != other.start_;
  }

  reference operator*() { return Ma_.get(); }

  pointer operator->() { return &Ma_.get(); }

  MaybeIterator& operator++() {
    start_ = false;
    return *this;
  }

  MaybeIterator& operator++(int) {
    MaybeIterator ret(Ma_, start_);
    start_ = false;
    return ret;
  }

 private:
  Maybe<A>& Ma_;
  bool start_;
};

/**
 * Immutable Iterator over Maybe.
 * @see MaybeIterator
 */
template <typename A> class ConstMaybeIterator {
 public:
  using value_type = const A;
  using reference = const A&;
  using pointer = const A*;

  ConstMaybeIterator(const Maybe<A>& Ma, bool start)
      : Ma_(Ma), start_(start && Ma.isJust()) {}

  bool operator!=(const ConstMaybeIterator<A>& other) {
    return start_ != other.start_;
  }

  reference operator*() { return Ma_.get(); }

  pointer operator->() { return &Ma_.get(); }

  ConstMaybeIterator& operator++() {
    start_ = false;
    return *this;
  }

  ConstMaybeIterator& operator++(int) {
    ConstMaybeIterator ret(Ma_, start_);
    start_ = false;
    return ret;
  }

 private:
  const Maybe<A>& Ma_;
  bool start_;
};

/**
 * Convenience constructor, copies/moves `a` into new Maybe instance.
 */
template <typename A> Maybe<std::decay_t<A>> Just(A&& a) {
  return Maybe<A>(std::forward<A>(a));
}
// @}
}  // namespace ma
