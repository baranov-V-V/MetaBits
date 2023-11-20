#pragma once

#include <value_types.hpp>


// Nats
namespace detail {
  
template <int N>
struct NatsHelper {
  using Head = value_types::ValueTag<N>;
  using Tail = NatsHelper<N + 1>;
};

} // namespace detail

using Nats = detail::NatsHelper<0>;


// Fib
namespace detail {
  
template <int LHS, int RHS>
struct FibHelper {
  using Head = value_types::ValueTag<LHS>;
  using Tail = FibHelper<RHS, LHS + RHS>;
};

} // namespace detail

using Fib = detail::FibHelper<0, 1>;


// Primes
namespace detail {

consteval bool IsPrime(int n) noexcept {
  for (int i = 2; i * i <= n; i++) {
    if (n % i == 0) {
      return false;
    }
  }
  return true;
}  

consteval int PrimeAt(int idx) noexcept {
  int num = 2;
  for (int i = 0; i < idx; ++num) {
    if (IsPrime(num)) {
      ++i;
    }
  }
  return num - 1;
}

template <int N>
struct PrimeHelper {
  using Head = value_types::ValueTag<PrimeAt(N)>;
  using Tail = PrimeHelper<N + 1>;
};

} // namespace detail

using Primes = detail::PrimeHelper<1>;
