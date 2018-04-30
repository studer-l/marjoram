#pragma once

#include "either.hpp"
#include <functional>
#include <type_traits>

namespace ma {
/**
 * @defgroup Lazy Lazy
 * @addtogroup Lazy
 * @{
 * Lazy Monad, supporting constants and functions.
 *
 * Represents a lazy `A` that will be obtained via a delayed computation. The
 * result will be stored for later re-use.
 *
 * Allows `const` access to the stored value, even if not yet computed (through
 * `mutable` internal storage).
 *
 * Example
 * -------
 *
 * Suppose we would like to simplify the following class:
 *
 * ~~~
 * class Example {
 *   public:
 *     void examplify() const {
 *       if (calcResult == -1) {
 *         calcResult = expensiveCalculation();
 *       }
 *       // use calcResult somehow
 *     }
 *
 *     void examplify2() const {
 *       if (calcResult == -1) {
 *         calcResult = expensiveCalculation();
 *       }
 *       // use calcResult in some other way
 *     }
 *
 *   private:
 *     double calcResult = -1; // -1 indicates not computed yet
 *     double expensiveCalculation();
 * };
 * ~~~
 *
 * We can do so using a `Lazy<Double>` as follows:
 * ~~~
 * class Example {
 *   public:
 *     Example() : calcResult([this](){ return expensiveCalculation(); }) {}
 *
 *     void examplify() const {
 *       // use calcResult somehow with calcResult.get() -> double
 *     }
 *
 *     void examplify2() const {
 *       // use calcResult in some other way with calcResult.get() -> double
 *     }
 *
 *   private:
 *     Lazy<double> calcResult;
 *     double expensiveCalculation();
 * };
 * ~~~
 */

template <class A> class LazyIterator;

/**
 * Lazy `A`; Either contains an `A` or a function that yields an `A`.
 */
template <typename A> class Lazy {
 public:
  using value_type = A;

  static_assert(std::is_same<std::decay_t<A>, A>::value,
                "Lazy<A>: A must be a value type.");

  /**
   * @param f Function that will be called exactly once when `get` is called
   * the first time.
   */
  explicit Lazy(std::function<A()> f) : impl(Left, f) {}
  explicit Lazy(const A& a) : impl(Right, a) {}

  Lazy(const Lazy&) = default;
  Lazy& operator=(const Lazy&) = default;

  Lazy& operator=(const A& a) {
    /* n.b. a new storage_t is made via copy before the old one is overwritten,
     * hence the case `Lazy<A> La(...); La = La.get();` is safe */
    impl = storage_t(Right, a);
    return *this;
  }

  Lazy& operator=(A&& a) {
    impl = storage_t(Right, std::move(a));
    return *this;
  }

  /**
   * @return true iff has been evaluated.
   */
  bool isEvaluated() const { return impl.isRight(); }

  /**
   * @return If the function has not yet been evaluated, evaluates it and
   * stores its value. Then returns the stored value.
   */
  A& get() {
    if (impl.isLeft()) {
      impl = storage_t(Right, impl.asLeft()());
    }
    return impl.asRight();
  }

  /**
   * @return If the function has not yet been evaluated, evaluates it and
   * stores its value. Then returns the stored value.
   */
  const A& get() const {
    if (impl.isLeft()) {
      impl = storage_t(Right, impl.asLeft()());
    }
    return impl.asRight();
  }

  /**
   * Composes self with `g`, that is, creates a new lazy value that yields the
   * result of composing `g` with `f`, essentially `Lazy<T>(g(f()))`.
   *
   * @param g Function object.
   *
   * Type requirement:
   * - `G::operator()` when called with argument of type `A&&` has non-void
   *   return type.
   *
   *   @return Lazy composition.
   *
   * @note The `g` is captured inside the returned lazy object by
   * reference and must persist until the first evaluation of the return
   * value.
   */
  template <typename G> auto map(G g) const -> Lazy<std::result_of_t<G(A)>> {
    return Lazy<std::result_of_t<G(A)>>([g, this]() { return g(get()); });
  }

  /**
   * Composes self with `g`, that is, creates a new lazy value that yields the
   * result of composing `g` with `f`, essentially `Lazy<T>(g(f()))`.
   *
   * @param g Function object.
   *
   * Type requirement:
   * - `G::operator()` when called with argument of type `A&&` has non-void
   *   return type.
   *
   *   @return Lazy composition.
   *
   * @note The `g` object is captured inside the returned lazy object by
   * reference and must persist until the first evaluation of the return
   * value.
   */
  template <typename G> auto map(G g) -> Lazy<std::result_of_t<G(A)>> {
    return Lazy<std::result_of_t<G(A)>>(
        [g, this]() mutable { return g(std::move(get())); });
  }

  /**
   * Composes self with `g`, that is, creates a new lazy value that yields the
   * result of composing `g` with `f` and flattening, essentially
   * `Lazy<T>(g(f()))`.
   *
   * @param g Function object.
   *
   * Type requirement:
   * - `G::operator()` when called with argument of type `A&&` has return type
   *   `Lazy<T>` for some type `T`.
   *
   *   @return Lazy composition.
   *
   * @note The `g` object is captured inside the returned lazy object by
   * reference and must persist until the first evaluation of the return
   * value.
   */
  template <typename G> auto flatMap(G g) const -> std::result_of_t<G(A)> {
    /* we need to artificially constraint the type here */
    using R = typename std::result_of_t<G(A)>::value_type;
    static_assert(std::is_same<Lazy<R>, std::result_of_t<G(A)>>::value,
                  "Type mismatch in F for Lazy<A>::flatMap(F: A -> Lazy<R>)");
    return std::result_of_t<G(A)>([g, this]() { return g(get()).get(); });
  }

  /**
   * Composes self with `g`, that is, creates a new lazy value that yields the
   * result of composing `g` with `f` and flattening, essentially
   * `Lazy<T>(g(f()))`.
   *
   * @param g Function object.
   *
   * Type requirement:
   * - `G::operator()` when called with argument of type `A&&` has return type
   *   `Lazy<T>` for some type `T`.
   *
   *   @return Lazy composition.
   *
   * @note The `g` object is captured inside the returned lazy object by
   * reference and must persist until the first evaluation of the return
   * value.
   */
  template <typename G> auto flatMap(G g) -> std::result_of_t<G(A)> {
    /* we need to artificially constraint the type here */
    using R = typename std::result_of_t<G(A)>::value_type;
    static_assert(std::is_same<Lazy<R>, std::result_of_t<G(A)>>::value,
                  "Type mismatch in F for Lazy<A>::flatMap(F: A -> Lazy<R>)");
    return std::result_of_t<G(A)>(
        [g, this]() mutable { return g(std::move(get())).get(); });
  }

  LazyIterator<A> begin() const { return LazyIterator<A>(*this, true); }
  LazyIterator<A> end() const { return LazyIterator<A>(*this, false); }

 private:
  using storage_t = Either<std::function<A()>, A>;
  mutable storage_t impl;
};

/**
 * Flattens a nested Lazy.
 * Note that the none of the underlying computations is triggered and the
 * returned lazy object is independent of the input.
 */
template <typename A> Lazy<A> Flatten(const Lazy<Lazy<A>>& LLa) {
  return Lazy<A>([LLa]() { return LLa.get().get(); });
}

/**
 * Flattens a nested Lazy.
 * Note that the none of the underlying computations is triggered and on
 * completion and the input is moved from.
 */
template <typename A> Lazy<A> Flatten(Lazy<Lazy<A>>&& LLa) {
  return Lazy<A>([nested = std::move(LLa)]() { return nested.get().get(); });
}

/**
 * Iterator over Lazy. Allows mutable access to value contained.
 * If the Lazy instance from which the iterator was created has not yet been
 * evaluated, it is evaluated once the iterator is dereferenced.
 *
 * Example:
 *
 * @code
 * Lazy<A> La;
 * [...]
 * for (const A& a: La) {
 *  // use `a`
 * }
 * @endcode
 */
template <typename A> class LazyIterator {
 public:
  using iterator_category = std::input_iterator_tag;
  using difference_type = size_t;
  using value_type = const A;
  using reference = const A&;
  using pointer = const A*;

  LazyIterator(const Lazy<A>& La, bool start) : La_(La), start_(start) {}

  bool operator!=(const LazyIterator<A>& other) {
    return start_ != other.start_;
  }

  reference operator*() { return La_.get(); }

  pointer operator->() { return &La_.get(); }

  LazyIterator& operator++() {
    start_ = false;
    return *this;
  }

  LazyIterator& operator++(int) {
    LazyIterator ret(La_, start_);
    start_ = false;
    return ret;
  }

 private:
  const Lazy<A>& La_;
  bool start_;
};
// @}
}  // namespace ma
