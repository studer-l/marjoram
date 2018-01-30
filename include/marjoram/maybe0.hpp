#pragma once

#include "maybe.hpp"

namespace ma {
namespace detail {
/**
 * Implementation of Maybe that wraps a pointer-like type.
 *
 * Note that this template by itself does not meet the type requirements for
 * `impl` in Maybe. To do this, Ptr must be fixed to a specific kind of pointer
 * (i.e. `std::unique_ptr<A>`, or plain old pointer `A*`). This is achieved via
 * template argument currying.
 */
template <class A, class Ptr> class MaybePtrImplT {
 public:
  static_assert(
      std::is_same<typename std::pointer_traits<Ptr>::element_type, A>::value,
      "Trying to create MaybePtrImplT with mismatching types");

  MaybePtrImplT() : p_(nullptr) {}
  // MaybePtrImplT(Ptr p) : p_(p) {}  @FIXME why can't this overload co-exist?
  MaybePtrImplT(Ptr&& p) : p_(std::move(p)) {}

  bool is_initialized() const { return !!p_; }
  A& get() { return *p_; }
  const A& get() const { return *p_; }

 private:
  Ptr p_;
};

/**
 * Fixes second template argument in
 * template <class A, class B>  to B0, obtaining essentially
 * template <class A, class B = B0>
 */
template <template <class...> class F, class... B> struct Curry {
  template <typename A> using type = F<A, B...>;
};
}  // namespace detail

/**
 * @defgroup MaybeNull MaybeNull
 *
 * Template class to wrap pointer-like types and treat them as `Maybe`. Use
 * `nullptr` as sentinel value to indicate Nothing.
 *
 * Downside: Cannot contain `nullptr`. Depending on Ptr, might not be copyable,
 * or copying might lead to dangling pointers. Use with care.
 *
 * Example
 * -------
 * Suppose `requestWidget` returned a pointer to `Widget`, where `nullptr`
 * indicates operation failed:
 *
 * ~~~
 * Widget* requestWidget(double length, int numberOfBells);
 * ~~~
 *
 * Then the received pointer-like type can be wrapped in a Maybe instance:
 *
 * ~~~
 * Maybe0_t<Widget*> mw = Maybe0(requestWidget(14.2, 3));
 * ~~~
 *
 * Note that no memory management is performed by the `Maybe0_t`, if memory should
 * be freed then a smart pointer should be used:
 *
 * ~~~
 * using wptr = std::unique_ptr<Widget>;
 * Maybe0_t<Widget*> mw = Maybe0(wptr(requestWidget(14.2, 3)));
 * ~~~
 */

template <class Ptr>
using Maybe0_t =
    Maybe<typename std::pointer_traits<Ptr>::element_type,
          detail::Curry<detail::MaybePtrImplT, Ptr>::template type>;

/**
 * @ingroup MaybeNull
 * Constructor, moves `p` into new Maybe0 instance.
 */
template <typename Ptr> Maybe0_t<Ptr> Maybe0(Ptr&& p) {
  return Maybe0_t<Ptr>(std::forward<Ptr>(p));
}
}  // namespace ma
