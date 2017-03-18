#pragma once

#include <forward_list>
#include <type_traits>

/**
 * Toplevel project namespace
 */
namespace marjoram {
/**
 * Monad trait
 *
 * Type Requirements:
 * - M<class> is a monad
 * - The currently contained type is A.
 *
 * Use with Curiously recurring template pattern, i.e.
 * \code
 * template <typename A> class Maybe : public Monad<A, Maybe> { ... };
 * \endcode
 *
 * Provides const and mutable map, based on `M<A>`'s flatMap and constructor
 * `M<B>(B)`.
 */
template <typename A, template <class> class M> class Monad {
 public:
  /**
   * Apply `f` to `a`, wrap result in M.
   *
   * @param f Function object.
   *
   * Type requirements:
   * - `F::operator()` when called with `const A&` argument  has return type
   *   `M<B>`, where `B` is non void
   *
   * @return `M<B>` containing the result of `M<B>(f(a))` or `Nothing`.
   */
  template <typename F> auto map(F f) {
    using B = std::result_of_t<F(std::decay_t<A>)>;
    return Ma().flatMap([f](A&& a) { return M<B>(f(std::move(a))); });
  }

  /**
   * Apply `f` to `a`, wrap result in M.
   *
   * @param f Function object.
   *
   * Type requirements:
   * - `F::operator()` when called with `A&&` argument  has return type
   *   `M<B>`, where `B` is non void
   *
   * @return `M<B>` containing the result of `M<B>(f(a))` or `Nothing`.
   */
  template <typename F> auto map(F f) const {
    using B = std::result_of_t<F(std::decay_t<A>)>;
    return Ma().flatMap([f](const A& a) { return M<B>(f(a)); });
  }

 private:
  M<A>& Ma() {
    static_assert(std::is_base_of<Monad<A, M>, M<A>>::value, "");
    return static_cast<M<A>&>(*this);
  }

  const M<A>& Ma() const {
    static_assert(std::is_base_of<Monad<A, M>, M<A>>::value, "");
    return static_cast<const M<A>&>(*this);
  }
};

/* free, monadic functions */

/**
 * map2
 *
 * F: (A, B) -> C
 */
template <typename A, typename B, template <class> class M, typename F>
auto map2(const M<A>& ma, const M<B>& mb, F f) -> M<std::result_of_t<F(A, B)>> {
  return ma.flatMap([&mb, f](const A& a) {
    return mb.map([&a, f](const B& b) { return f(a, b); });
  });
}

/**
 * Sequence
 *
 * Sequence Cont[M[A]] -> M[List[A]]
 */
template <typename A, template <class> class M, template <class...> class Cont>
auto sequence(Cont<M<A>>& cma) -> M<std::forward_list<A>> {
  M<std::forward_list<A>> ret((std::forward_list<A>()));
  for (M<A>& ma : cma) {
    ret = map2(ret, ma, [](std::forward_list<A> la, A a) {
      la.push_front(a);
      return la;
    });
  }
  return ret;
}
}  // namespace marjoram
