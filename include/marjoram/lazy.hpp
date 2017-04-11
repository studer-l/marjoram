#pragma once

#include "either.hpp"
#include <functional>
#include <type_traits>

namespace marjoram {

/**
 * Lazy `A`; Either contains an `A` or a function that yields an `A`.
 */
template <typename A> class Lazy {
 public:
  using value_type = A;

  /**
   * @param f Function that will be called exactly once when `get` is called
   * the first time.
   */
  Lazy(std::function<A()> f) : impl(LeftEither, f) {}
  Lazy(const A& a) : impl(RightEither, a) {}

  Lazy(const Lazy&) = default;
  Lazy& operator=(const Lazy&) = default;

  Lazy& operator=(const A& a) {
    /* n.b. a new storage_t is made via copy before the old one is overwrriten,
     * hence the case `Lazy<A> Ma(...); Ma = Ma.get();` is safe */
    impl = storage_t(RightEither, a);
    return *this;
  }

  Lazy& operator=(A&& a) {
    impl = storage_t(RightEither, std::move(a));
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
      impl = storage_t(RightEither, impl.asLeft()());
    }
    return impl.asRight();
  }

  /**
   * @return If the function has not yet been evaluated, evaluates it and
   * stores its value. Then returns the stored value.
   */
  const A& get() const {
    if (impl.isLeft()) {
      impl = storage_t(RightEither, impl.asLeft()());
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
   *   Note that this object is captured inside the returned lazy object by
   *   reference and must persist until the first evaluation of the return
   *   value.
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
   *   Note that this object is captured inside the returned lazy object by
   *   reference and must persist until the first evaluation of the return
   *   value.
   */
  template <typename G> auto map(G g) -> Lazy<std::result_of_t<G(A)>> {
    return Lazy<std::result_of_t<G(A)>>(
        [g, this]() mutable { return g(std::move(get())); });
  }

  /**
   * Composes self with `g`, that is, creates a new lazy value that yields the
   * result of composing `g` with `f` nad flattening, essentially
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
   *   Note that this object is captured inside the returned lazy object by
   *   reference and must persist until the first evaluation of the return
   *   value.
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
   * result of composing `g` with `f` nad flattening, essentially
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
   *   Note that this object is captured inside the returned lazy object by
   *   reference and must persist until the first evaluation of the return
   *   value.
   */
  template <typename G> auto flatMap(G g) -> std::result_of_t<G(A)> {
    /* we need to artificially constraint the type here */
    using R = typename std::result_of_t<G(A)>::value_type;
    static_assert(std::is_same<Lazy<R>, std::result_of_t<G(A)>>::value,
                  "Type mismatch in F for Lazy<A>::flatMap(F: A -> Lazy<R>)");
    return std::result_of_t<G(A)>(
        [g, this]() mutable { return g(std::move(get())).get(); });
  }

 private:
  using storage_t = marjoram::Either<std::function<A()>, A>;
  mutable storage_t impl;
};

/**
 * Flattens a nested Lazy.
 * Note that the none of the underlying computations is triggered and the
 * returned lazy object is independent of the input.
 */
template <typename A> Lazy<A> Flatten(const Lazy<Lazy<A>>& LLa) {
  return Lazy<A>([LLa](){ return LLa.get().get(); });
}


/**
 * Flattens a nested Lazy.
 * Note that the none of the underlying computations is triggered and on
 * completion and the input is moved from.
 */
template <typename A> Lazy<A> Flatten(Lazy<Lazy<A>>&& LLa) {
  return Lazy<A>([nested = std::move(LLa)](){ return nested.get().get(); });
}
}
