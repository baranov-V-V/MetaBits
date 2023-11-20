#include <cassert>
#include <iterator>
#include <limits>
#include <memory>
#include <ranges>

namespace detail {

constexpr auto DynamicExtent = std::numeric_limits<std::size_t>::max();

template <std::size_t size>
class SpanBase {
 public:
  constexpr SpanBase(const std::size_t) noexcept {}

  constexpr auto Size() const noexcept {
    return size;
  }
};

template <>
class SpanBase<DynamicExtent> {
 public:

  constexpr SpanBase(const std::size_t size) noexcept : size_{size} {}

  constexpr auto Size() const noexcept {
    return size_;
  }

 private:
  std::size_t size_;
};

}

template <typename T, std::size_t size = detail::DynamicExtent>
class Span : private detail::SpanBase<size> {
 public:
  using element_type = T;
  using value_type = std::remove_cv_t<element_type>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using iterator = pointer;
  using reverse_iterator = std::reverse_iterator<iterator>;

 public:
  constexpr Span() noexcept
    : detail::SpanBase<size>(0)
    , data_{nullptr} {
  }

  template <typename Iter>
  constexpr Span(Iter iter, std::size_t values_count) noexcept
    : detail::SpanBase<size>(values_count)
    , data_{std::to_address(iter)} {
  }

  template <typename Iter>
  constexpr Span(Iter first, Iter last)
    : detail::SpanBase<size>(std::distance(first, last))
    , data_{std::to_address(first)} {
  }

  template <typename TArr, std::size_t array_size>
  constexpr Span(TArr (&array)[array_size]) noexcept
    : detail::SpanBase<size>(array_size)
    , data_{static_cast<pointer>(std::data(array))} {
  }

  template <typename TArr, std::size_t array_size>
  constexpr Span(std::array<TArr, array_size>& array) noexcept
    : detail::SpanBase<size>(array_size)
    , data_{static_cast<pointer>(std::data(array))} {
  }

  template <typename TArr, std::size_t array_size>
  constexpr Span(const std::array<TArr, array_size>& array) noexcept
    : detail::SpanBase<size>(array_size)
    , data_{static_cast<pointer>(std::data(array))} {
  }

  template <typename Range>
  constexpr Span(Range&& range)
    : Span{std::ranges::begin(range), std::ranges::end(range)} {
  }

  constexpr reference Front() const noexcept {
    return data_[0];
  }

  constexpr reference Back() const noexcept {
    return data_[Size() - 1];
  }

  constexpr reference operator[](std::size_t index) const noexcept {
    assert(index < Size());
    return data_[index];
  }

  constexpr pointer Data() const noexcept {
    return data_;
  }

  template <std::size_t count>
  constexpr Span<element_type, count> First() const noexcept {
    assert(count <= Size());
    return Span<element_type, count>{Data(), count};
  }

  template <size_t count>
  constexpr Span<element_type, count> Last() const noexcept {
    assert(count <= Size());
    return {Data() + Size() - count, count};
  }

  constexpr Span<element_type, detail::DynamicExtent> First(size_type count) const noexcept {
    assert(count <= Size());
    return {Data(), count};
  }

  constexpr Span<element_type, detail::DynamicExtent> Last (size_type count) const noexcept {
    assert(count <= Size());
    return {Data() + Size() - count, count};
  }

  constexpr std::size_t Size() const noexcept {
    return detail::SpanBase<size>::Size();
  }

  constexpr auto SizeBytes() const noexcept {
    return sizeof(T) * Size();
  }

  constexpr auto Empty() const noexcept {
    return this->Size() == 0;
  }

  constexpr iterator begin() const noexcept {
    return Data();
  }

  constexpr iterator end() const noexcept {
    return Data() + Size();
  }

  constexpr reverse_iterator rbegin() const noexcept {
    return reverse_iterator{end()};
  }

  constexpr auto rend() const noexcept {
    return reverse_iterator{begin()};
  }

 private:
  T* data_;
};

template <typename Iter>
Span(Iter, std::size_t) -> Span<std::iter_value_t<Iter>, detail::DynamicExtent>;

template <typename T, std::size_t size>
Span(T (&)[size]) -> Span<T, size>;

template <typename T, std::size_t size>
Span(std::array<T, size>&) -> Span<T, size>;

template <typename T, std::size_t size>
Span(const std::array<T, size>&) -> Span<const T, size>;

template <typename Range>
Span(Range&&) -> Span<std::ranges::range_value_t<Range>, detail::DynamicExtent>;