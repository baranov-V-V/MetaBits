#pragma once

#include <concepts>

#include "type_tuples.hpp"

namespace type_lists {

template<class TL>
concept TypeSequence =
  requires {
    typename TL::Head;
    typename TL::Tail;
  };

struct Nil {};

template<class TL>
concept Empty = std::derived_from<TL, Nil>;

template<class TL>
concept TypeList = Empty<TL> || TypeSequence<TL>;


// Cons
template <class T, TypeList TL>
struct Cons {
  using Head = T;
  using Tail = TL;
};


// FromTuple
namespace detail {

template<class TT>
struct FromTupleHelper;

template<class T, class... Ts>
struct FromTupleHelper<type_tuples::TTuple<T, Ts...>> {
  using Head = T;
  using Tail = FromTupleHelper<type_tuples::TTuple<Ts...>>;
};

template<>
struct FromTupleHelper<type_tuples::TTuple<>> : Nil {};
} // namespace detail

template<class TT>
using FromTuple = detail::FromTupleHelper<TT>;


// ToTuple
namespace detail {

template<TypeList TL, class... Ts>
struct ToTuple;

template <TypeSequence TS, class... Ts>
struct ToTuple<TS, Ts...> {
  using value = typename ToTuple<typename TS::Tail, Ts..., typename TS::Head>::value;
};

template <Empty E, class... Ts>
struct ToTuple<E, Ts...> {
  using value  = type_tuples::TTuple<Ts...>;
};

} // namespace detail

template<TypeList TL>
using ToTuple = typename detail::ToTuple<TL>::value;


// Repeat
template <class T>
struct Repeat {
  using Head = T;
  using Tail = Repeat<T>;
};


// Take
template <std::size_t N, class T>
struct Take : Nil {};

template <std::size_t N, TypeSequence TS> requires(N > 0)
struct Take<N, TS> {
  using Head = TS::Head;
  using Tail = Take<N - 1, typename TS::Tail>;
};


// Drop
template <std::size_t N, TypeList TL>
struct Drop : TL {};

template <std::size_t N, TypeSequence TS> requires(N > 0)
struct Drop<N, TS> : Drop<N - 1, typename TS::Tail> {};


// Replicate
template <std::size_t N, class T>
struct Replicate {
  using Head = T;
  using Tail = Replicate<N - 1, T>;
};

template <class T>
struct Replicate<0, T> : Nil {};


// Map
namespace detail {
  
template <template<class> typename Mapper, TypeList TL>
struct MapHelper;

template <template<class> typename Mapper, TypeSequence TS>
struct MapHelper<Mapper, TS> {
  using Head = Mapper<typename TS::Head>;
  using Tail = MapHelper<Mapper, typename TS::Tail>;
};

template <template<class> typename Mapper, Empty E>
struct MapHelper<Mapper, E> : Nil {};

} // namespace detail

template <template<class> typename Mapper, TypeList TL>
using Map = detail::MapHelper<Mapper, TL>;


// Filter
namespace detail {


template<template<class> typename Predicate, class T>
concept PredicateTrue = requires() {
  requires Predicate<T>::Value == true;
};

template <template<class> typename Predicate, TypeList TL>
struct FilterHelper;

template <template<class> typename Predicate, TypeSequence TS>
requires(Predicate<typename TS::Head>::Value == true)
struct FilterHelper<Predicate, TS> {
  using Head = TS::Head;
  using Tail = FilterHelper<Predicate, typename TS::Tail>;
};

template <template<class> typename Predicate, TypeSequence TS>
struct FilterHelper<Predicate, TS> : FilterHelper<Predicate, typename TS::Tail> {};

template <template<class> typename Predicate, Empty E>
struct FilterHelper<Predicate, E> : Nil {};

} // namespace detail

template <template<class> typename Predicate, TypeList TL>
using Filter = detail::FilterHelper<Predicate, TL>;


// Iterate 
template <template<class> typename Func, class T>
struct Iterate {
  using Head = T;
  using Tail = Iterate<Func, Func<T>>;
};


// Cycle
namespace detail {

template <TypeList Curr, TypeList Full>
struct CycleHelper;

template <TypeList Curr, Empty Full>
struct CycleHelper<Curr, Full> : Nil {};

template <TypeSequence Curr, TypeSequence Full>
struct CycleHelper<Curr, Full> {
  using Head = Curr::Head;
  using Tail = CycleHelper<typename Curr::Tail, Full>;
};

template <Empty Curr, TypeSequence Full>
struct CycleHelper<Curr, Full> : CycleHelper<Full, Full>{};

} // namespace detail

template <TypeList TL>
using Cycle = detail::CycleHelper<TL, TL>;


// Inits
namespace detail {
  
template <TypeList TL, class T>
struct Append;

template <TypeSequence TS, class T>
struct Append<TS, T> {
  using Head = TS::Head;
  using Tail = Append<typename TS::Tail, T>;
};

template <Empty E, class T>
struct Append<E, T> {
  using Head = T;
  using Tail = Nil;
};

template <TypeList LHS, TypeList RHS>
struct InitsHelper;

template <TypeList LHS, TypeSequence RHS>
struct InitsHelper<LHS, RHS> {
  using Head = LHS;
  using Tail = InitsHelper<Append<LHS, typename RHS::Head>, typename RHS::Tail>;
};

template <TypeList LHS, Empty RHS>
struct InitsHelper<LHS, RHS> {
  using Head = LHS;
  using Tail = Nil;
};

} // namespace detail

template <TypeList TL>
using Inits = detail::InitsHelper<Nil, TL>;


// Tails
namespace detail {

template <TypeList TL>
struct TailsHelper;

template <TypeSequence TS>
struct TailsHelper<TS> {
  using Head = TS;
  using Tail = TailsHelper<typename TS::Tail>;
};

template <Empty T>
struct TailsHelper<T> {
  using Head = Nil;
  using Tail = Nil;
};

}; // namespace detail

template <TypeList TL>
using Tails = detail::TailsHelper<TL>;


// Scanl
namespace detail {

template <template<class, class> typename OP, class T, TypeList TL>
struct ScanlHelper;

template <template<class, class> typename OP, class T, TypeSequence TS>
struct ScanlHelper<OP, T, TS> {
  using Head = T;
  using Tail = ScanlHelper<OP, OP<T, typename TS::Head>, typename TS::Tail>;
};

template <template<class, class> typename Op, class T, Empty E>
struct ScanlHelper<Op, T, E> {
  using Head = T;
  using Tail = Nil;
};

} // namespace detail

template <template<class, class> typename OP, class T, TypeList TL>
using Scanl = detail::ScanlHelper<OP, T, TL>;


// Foldl
namespace detail {

template <TypeList TL>
struct FoldlHelper;

template <TypeSequence TS>
requires TypeSequence<typename TS::Tail>
struct FoldlHelper<TS> {
  using value = typename FoldlHelper<typename TS::Tail>::value;
};

template <TypeSequence TS>
struct FoldlHelper<TS> {
  using value = TS::Head;
};

} // namespace detail

template <template<class, class> typename OP, class T, TypeList TL>
using Foldl = detail::FoldlHelper<Scanl<OP, T, TL>>::value;


// Zip2
namespace detail {
  
template <TypeList LHS, TypeList RHS>
struct Zip2Helper;

template <TypeSequence LHS, TypeSequence RHS>
struct Zip2Helper<LHS, RHS> {
  using Head = type_tuples::TTuple<typename LHS::Head, typename RHS::Head>;
  using Tail = Zip2Helper<typename LHS::Tail, typename RHS::Tail>;
};

template <Empty LHS, TypeList RHS>
struct Zip2Helper<LHS, RHS> : Nil {};

template <TypeList LHS, Empty RHS>
struct Zip2Helper<LHS, RHS> : Nil {};

} // namespace detail

template <TypeList LHS, TypeList RHS>
using Zip2 = detail::Zip2Helper<LHS, RHS>;


// Zip
namespace detail {

template <class... Ts>
struct ZipHelper;

template <TypeSequence... Ts>
struct ZipHelper<Ts...> {
  using Head = type_tuples::TTuple<typename Ts::Head...>;
  using Tail = ZipHelper<typename Ts::Tail...>;
};

template <class... Ts>
concept HasEmpty = requires {
  requires (std::is_same_v<Nil, Ts> || ...) == true;
};

template <HasEmpty... Ts>
struct ZipHelper<Ts...> : Nil {};

} // namespace detail

template <TypeList... Ts>
using Zip = detail::ZipHelper<Ts...>;

} // namespace type_lists
