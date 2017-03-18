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

### Monad
```c++
template <typename A, template <class> class M> class Monad
```

A Monad _mixin_ (using
[CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern)),
providing `map` in terms of `flatMap` and `M(A)`. Also acts as an interface to
other functions, such as `map2` and `sequence`.

Example usage for `Maybe`:
```c++
template <typename A> class Maybe : public Monad<A, Maybe> { ... }
```
