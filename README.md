# Marjoram

[![Build Status](https://travis-ci.org/studer-l/marjoram.svg?branch=master)](https://travis-ci.org/studer-l/marjoram)
[![Coverage Status](https://coveralls.io/repos/github/studer-l/marjoram/badge.svg?branch=coveralls)](https://coveralls.io/github/studer-l/marjoram?branch=coveralls)

Tiny header-only library for functional programming in C++.

## Requirements

- Modern C++ compiler (C++14 or newer)
- Boost 1.58 or newer (for move-only optional)

## Components

Marjoram provides several convenient templates, mainly:

* [Maybe](#maybe)
* [Maybe0](#maybe0)
* [Either](#either)
* [Lazy](#lazy)
* [Reader](#reader)


### Maybe
```c++
template <class T> class Maybe;
```
Straight up extension to `std::optional`/`boost::optional`, endowing it with
`flatMap`.

#### Example usage
```c++
  class BetterWidget;
  class Widget {
    BetterWidget morph();
  };

  Maybe<Widget> requestWidget(double length, int numberOfBells) {
    /* do some complicated thing */
    if (/* failure */ ) {
      return Nothing;
    }
    return widget;
  }
```

The return value of `requestWidget` can be used regardless of whether the
operation succeeded by using `map`:

```c++
auto mw = requestWidget(14.2, 3);
/* if a widget was created, morph it  */
Maybe<BetterWidget> morphed = mw.map([](Widget& w) { return w.morph(); });
```

### Maybe0

Template class to wrap pointer-like types and treat them as `Maybe`.

#### Example usage

Suppose `requestWidget` returned a pointer to `Widget`, where `nullptr`
indicates operation failed:

```c++
Widget* requestWidget(double length, int numberOfBells);
```

Then the example above can be rewritten as:

```c++
Maybe0_t<Widget*> mw = Maybe0(requestWidget(14.2, 3));
/* if a widget was created, morph it  */
Maybe<BetterWidget> morphed = mw.map([](Widget& w) { return w.morph(); });
```

Note that no memory management is performed by the `Maybe0_t`, if memory should
be freed then a smart pointer should be used:

```c++
using wptr = std::unique_ptr<Widget>;
Maybe0_t<Widget*> mw = Maybe0(wptr(requestWidget(14.2, 3)));
/* if a widget was created, morph it  */
Maybe<BetterWidget> morphed = mw.map([](Widget& w) { w.morph(); });
```

### Either
```c++
template <class L, class R> class Either;
```
Contains either an `L` or an `R`. Unlike `std::variant` both `L` and `R` may be
the same type.

#### Example usage

```c++
/*
 * @return Either a fully fledged Widget or a descriptive error message.
 */
Either<std::string, Widget> requestWidget(double length, int numberOfBells);
```

Then client code can treat the above case uniformly using `map` again, as
above with `Maybe`:

```c++
auto emw = requestWidget(14.2, 3);
/* if a widget was created, morph it  */
Either<std::string, BetterWidget> morphed =
                                 emw.map([](Widget& w) { return  w.morph(); });
```

Note that `Either`, `Maybe` and `Maybe0` can be used with non-copyable types,
possibly allowing them to be moved out of the container.


### Lazy
```c++
template <class T> Lazy;
```

Represents a lazy `A` that will be obtained via a delayed computation. The
result will be stored for later re-use.

Allows `const` access to the stored value, even if not yet computed (through
`mutable` internal storage).

#### Example usage

Suppose we would like to simplify the following class:

```c++
class Example {
  public:
    void examplify() const {
      if (calcResult == 0) {
        calcResult = expensiveCalculation();
      }
      /* use calcResult somehow */
    }

    void examplify2() const {
      if (calcResult == 0) {
        calcResult = expensiveCalculation();
      }
      /* use calcResult in some other way */
    }

  private:
    double calcResult = -1; // -1 indicates not computed yet
    double expensiveCalculation();
};
```

We can do so using a `Lazy<Double>` as follows:

```c++
class Example {
  public:
    Example() : calcResult([this](){ return expensiveCalculation(); }) {}

    void examplify() const {
      /* use calcResult somehow with calcResult.get() -> double */
    }

    void examplify2() const {
      /* use calcResult in some other way with calcResult.get() -> double */
    }

  private:
    Lazy<double> calcResult;
    double expensiveCalculation();
};
```

### Reader
```c++
template <class A, class R> Reader;
```

Represents computation that requires a shared resource `A` to run. Can be used
to implement Dependency Injection (see `test/test_reader.cxx` and [this
blogpost](http://blog.originate.com/blog/2013/10/21/reader-monad-for-dependency-injection/)
for illustration).
