# Marjoram
Tiny library for Functional Programming in C++

## Requirements

- Modern C++ compiler (C++14 or newer)
- Boost 1.58 or newer (for move-only optional)

## Components

### Maybe
```c++
template <class T> class Maybe<T>;
```
Straight up extension to `std::optional`/`boost::optional`, endowing it with
`flatMap`, thus turning it into a proper monad.

### Either
```c++
template <class L, class R> class Either<L, R>;
```
Another monadic datatype; Unlike `std::variant` both `L` and `R` may be the
same type.

Both `Either` and `Maybe` can be used with non-copyable types, possibly
allowing them to be moved out of the container.
