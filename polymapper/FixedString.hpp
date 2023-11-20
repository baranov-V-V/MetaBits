#pragma once

#include <string_view>
#include <cassert>
#include <array>

template<std::size_t max_length>
struct FixedString {
  constexpr FixedString(const char* string, std::size_t length) noexcept
    : data_()
    , size_(length) {
    assert(string != nullptr);
    assert(length <= max_length);

    std::copy(string, string + length, std::begin(data_));
  }

  constexpr operator std::string_view() const noexcept{
    return {std::data(data_), size_};
  }

  std::array<char, max_length> data_;
  std::size_t size_;
};

constexpr FixedString<256> operator""_cstr(const char* data, std::size_t size) noexcept {
  return {data, size};
}


/*
template <std::size_t capacity>
struct FixedString {
  constexpr FixedString(const char* data, std::size_t size) noexcept
    : data_{}
    , size_{size} {
    assert(data != nullptr);
    assert(size < capacity);

    for (auto i = std::size_t{}; i < size; ++i) {
      data_[i] = data[i];
    }
  }

  constexpr operator std::string_view() const noexcept {
    return std::string_view{data_, size_};
  }

  char data_[capacity];
  std::size_t size_;
};

constexpr FixedString<256> operator""_cstr(const char* data, std::size_t size) {
  return FixedString<256>{data, size};
}
*/