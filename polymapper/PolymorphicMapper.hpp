#pragma once

#include <optional>
#include <memory>

namespace detail {

template <typename Mapping, typename Target>
concept IsMapping = requires {
  { Mapping::Target() } -> std::same_as<Target>;
};  

} // namespace detail

template <typename From, auto target>
struct Mapping {
  using Base = From;

  static consteval auto Target() noexcept {
    return target;
  }
};

template <typename Base, typename Target, typename... Mappings>
requires (detail::IsMapping<Mappings, Target> && ...)
struct PolymorphicMapper;

template <typename Base, typename Target>
struct PolymorphicMapper<Base, Target>  {
  static std::optional<Target> map(const Base&) noexcept {
    return std::nullopt;
  }
};

template <typename Base, typename Target, typename Mapping, typename... Mappings>
struct PolymorphicMapper<Base, Target, Mapping, Mappings...> {
  static std::optional<Target> map(const Base& base) {
    if (dynamic_cast<const typename Mapping::Base*>(std::addressof(base))) {
      return Mapping::Target();
    }
    return PolymorphicMapper<Base, Target, Mappings...>::map(base);
  }
};
