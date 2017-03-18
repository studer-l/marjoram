#pragma once

#include <algorithm>
#include <cassert>
#include <new>

namespace marjoram {
/**
 * Tag indicating the left side.
 */
struct LeftSide {
  LeftSide() {}
};
/**
 * Convenience value for left side tag.
 */
static const LeftSide LeftEither;

/**
 * Tag indicating the right side.
 */
struct RightSide {
  RightSide() {}
};
/**
 * Convenience value for right side tag.
 */
static const RightSide RightEither;

namespace detail {

enum EitherSide : char { left, right };

/**
 * Implementation of (safe) union of two types.
 */
template <typename Left_t, typename Right_t> class EitherImpl {
 public:
  /**
   * Construct containing left type.
   */
  template <typename... Args>
  explicit EitherImpl(LeftSide /* selects overload */, Args&&... args)
      : side(EitherSide::left) {
    new (&storage) Left_t(std::forward<Args>(args)...);
  }

  /**
   * Construct containing right type.
   */
  template <typename... Args>
  explicit EitherImpl(RightSide /* selects overload */, Args&&... args)
      : side(EitherSide::right) {
    new (&storage) Right_t(std::forward<Args>(args)...);
  }

  /**
   * Destructor, calling destructor of contained type.
   */
  ~EitherImpl() { cleanup(); }

  /**
   * Copy constructor; Copies underlying value.
   */
  EitherImpl(const EitherImpl& rhs) : side(rhs.side) {
    if (side == left) {
      new (&storage) Left_t(rhs.asLeft());
    } else {
      new (&storage) Right_t(rhs.asRight());
    }
  }

  /**
   * Move constructor; Moves underlying value.
   */
  EitherImpl(EitherImpl&& rhs) noexcept : side(rhs.side) {
    if (side == left) {
      new (&storage) Left_t(std::move(rhs.asLeft()));
    } else {
      new (&storage) Right_t(std::move(rhs.asRight()));
    }
  }

  /**
   * Copy assignment operator; Copies underlying value.
   */
  EitherImpl& operator=(const EitherImpl& rhs) {
    /* destroy whatever we are currently holding */
    cleanup();

    /* copy rhs */
    side = rhs.side;
    if (side == left) {
      new (&storage) Left_t((rhs.asLeft()));
    } else {
      new (&storage) Right_t((rhs.asRight()));
    }
    return *this;
  }

  /**
   * Move assignment operator; Moves underlying value.
   */
  EitherImpl& operator=(EitherImpl&& rhs) noexcept {
    /* destroy whatever we are currently holding */
    cleanup();

    /* move rhs */
    side = rhs.side;
    if (side == left) {
      new (&storage) Left_t(std::move(rhs.asLeft()));
    } else {
      new (&storage) Right_t(std::move(rhs.asRight()));
    }
    return *this;
  }

  /**
   * Returns stored B value.
   *
   * Undefined behavior if this either does not contain a B.
   * @return reference to stored value of type B.
   */
  Right_t& asRight() {
    assert(side == right);
    return *reinterpret_cast<Right_t*>(&storage);
  }

  /**
   * Returns stored B value.
   *
   * Undefined behavior if this either does not contain a B.
   * @return reference to stored value of type B.
   */
  Right_t& asRight() const {
    assert(side == right);
    return *reinterpret_cast<const Right_t*>(&storage);
  }

  /**
   * Returns stored A value.
   *
   * Undefined behavior if this either does not contain a A.
   * @return reference to stored value of type A.
   */
  Left_t& asLeft() {
    assert(side == left);
    return *reinterpret_cast<Left_t*>(&storage);
  }

  /**
   * Returns stored A value.
   *
   * Undefined behavior if this either does not contain a A.
   * @return reference to stored value of type A.
   */
  Left_t& asLeft() const {
    assert(side == left);
    return *reinterpret_cast<const Left_t*>(&storage);
  }

 protected:
  /**
   * Indicates whether this contains an A or a B.
   */
  EitherSide side;

 private:
  void cleanup() {
    if (side == left) {
      asLeft().~Left_t();
    } else {
      asRight().~Right_t();
    }
  }

  using storage_t = typename std::aligned_union<1, Left_t, Right_t>::type;
  storage_t storage;
};
}  // namespace detail
}  // namespace marjoram
