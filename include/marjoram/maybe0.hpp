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
 * @defgroup Maybe0 Maybe0
 * @ingroup Maybe0
 * `Maybe0_t` --- MaybeNull
 * A `Maybe` backed by a pointer-like storage, for that Proper C-Like
 * Performance. `nullptr` is used as `Nothing`.
 *
 * Downside: Cannot contain nullptr. Depending on Ptr, might not be copyable,
 * or copying might lead to dangling pointers. Use with care.
 */
template <class Ptr>
using Maybe0_t =
    Maybe<typename std::pointer_traits<Ptr>::element_type,
          detail::Curry<detail::MaybePtrImplT, Ptr>::template type>;

/**
 * @ingroup Maybe0
 * Convenience constructor, moves `p` into new Maybe0 instance.
 */
template <typename Ptr> Maybe0_t<Ptr> Maybe0(Ptr&& p) {
  return Maybe0_t<Ptr>(std::forward<Ptr>(p));
}
}  // namespace ma
