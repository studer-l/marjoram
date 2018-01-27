#pragma once

#include <functional>

namespace ma {
/**
 * @defgroup Reader Reader
 * @addtogroup Reader
 * @{
 * Reader Monad, supporting constants and functions.
 */

/**
 * Reader Monad
 *
 * Essentially std::function with sugar on top.
 *
 * Represents computations that require a shared resource `A` to run.
 * map/flatMap composes further computations.
 */
template <class A, class R> class Reader {
 public:
  Reader(std::function<R(A)> f) : f_(f) {}

  /**
   * Run the function.
   */
  R run(const A& a) const { return f_(a); }

  /**
   * @param f Function object.
   *
   * Type requirement:
   * - `F::operator()` when called with `const R&` argument has non-void return
   *    type `C`.
   *
   * @return `Reader<A, C>`.
   */
  template <typename F>
  auto map(F f) const -> Reader<A, std::result_of_t<F(R)>> {
    return Reader<A, std::result_of_t<F(R)>>(
        [ f, self = *this ](const A& a) { return f(self.run(a)); });
  }

  /**
   * @param f Function object.
   *
   * Type requirement:
   * - `F::operator()` when called with `const R&` argument has return type
   *   `Reader<A, C>`, where `C` is non void
   *
   * @return `Reader<A, C>`.
   */
  template <typename F> auto flatMap(F f) -> std::result_of_t<F(R)> {
    using ReaderAC = std::result_of_t<F(R)>;
    using C = decltype(std::declval<ReaderAC>().run(std::declval<A>()));
    static_assert(std::is_same<ReaderAC, Reader<A, C>>::value,
                  "Reader::flatMap f type mismatch.");
    return Reader<A, C>(
        [ f, self = *this ](const A& a) { return f(self.run(a)).run(a); });
  }

 private:
  const std::function<R(A)> f_;
};
// @}
}  // namespace ma
