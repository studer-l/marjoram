#pragma once

namespace marjoram {
/**
 * Generic visitor from lambdas; Happens to match boost::static_visitor.
 * From http://articles.emptycrate.com/2017/02/06/why_inherit_from_lambdas.html
 */
template <typename R, typename... Fs> struct visitor : public Fs... {
  /**
   * Construct from lambdas.
   */
  /* implicit */visitor(Fs... fs) : Fs(fs)... {}

  /**
   * Result type when applying this visitor. Not verified.
   */
  using result_type = R;
};

/**
 * Convenience function to generate visitors from lambdas.
 */
template <typename R, typename... Fs> auto make_visitor(Fs... fs) {
  return visitor<R, Fs...>(fs...);
}
} // namespace marjoram
