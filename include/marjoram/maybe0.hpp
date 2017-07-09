#pragma once

#include "maybe.hpp"

namespace ma {
namespace detail {
/**
 * Implementation of Maybe that wraps a pointer-like type.
 *
 * Note that this is not a proper impl yet: The Ptr class has to be fixed for
 * this.
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
 * `Maybe0` --- MaybeNull
 * A `Maybe` backed by a pointer-like storage, for that Proper C-Like
 * Performance. `nullptr` is used as `Nothing`.
 *
 * Downside: Cannot contain nullptr. Depending on Ptr, might not be copyable,
 * or copying might lead to dangling pointers.
 */
template <class Ptr>
using Maybe0 = Maybe<typename std::pointer_traits<Ptr>::element_type,
                     detail::Curry<detail::MaybePtrImplT, Ptr>::template type>;

/**
 * Convenience constructor, moves `p` into new Maybe0 instance.
 */
template <typename Ptr> Maybe0<Ptr> Just0(Ptr&& p) {
  return Maybe0<Ptr>(std::forward<Ptr>(p));
}
}  // namespace ma
