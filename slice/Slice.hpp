#include <span>
#include <concepts>
#include <cstdlib>
#include <array>
#include <iterator>
#include <cassert>
#include <compare>
#include <limits>
#include <ranges>

inline constexpr std::ptrdiff_t dynamic_stride = -1;
inline constexpr std::size_t dynamic_extent = std::numeric_limits<std::size_t>::max();

namespace detail {

template <std::size_t size>
class SizeBase {
 public:
  constexpr SizeBase() noexcept = default;
  constexpr SizeBase(const std::size_t) noexcept {}

  constexpr auto Size() const noexcept {
    return size;
  }
};

template <>
class SizeBase<dynamic_extent> {
 public:
  constexpr SizeBase() noexcept = default;
  constexpr SizeBase(const std::size_t size) noexcept : size_{size} {}

  constexpr auto Size() const noexcept {
    return size_;
  }

 private:
  std::size_t size_;
};

template <std::ptrdiff_t stride>
class StrideBase {
 public:
  constexpr StrideBase() noexcept = default;
  constexpr StrideBase(const std::ptrdiff_t) noexcept {}

  constexpr auto Stride() const noexcept {
    return stride;
  }
};

template <>
class StrideBase<dynamic_stride> {
 public:
  constexpr StrideBase() noexcept = default;
  constexpr StrideBase(const std::ptrdiff_t stride) noexcept : stride_{stride} {}

  constexpr auto Stride() const noexcept {
    return stride_;
  }

 private:
  std::ptrdiff_t stride_;
};

inline constexpr std::size_t EffectiveSize(std::size_t count, std::size_t stride) {
  return count * stride;
}

} // namespace detail

template <class T, std::ptrdiff_t stride>
class SliceIterator : private detail::StrideBase<stride> {
 private:
  using StrideBase = detail::StrideBase<stride>;

 public:
  using iterator_concept [[maybe_unused]] = std::random_access_iterator_tag;
  using iterator_category = std::random_access_iterator_tag;
  using value_type        = std::remove_cv_t<T>;
  using difference_type   = std::ptrdiff_t;
  using pointer           = T*;
  using reference         = T&;

  constexpr SliceIterator() noexcept = default;

  constexpr SliceIterator(T* data, std::ptrdiff_t step) noexcept
    : StrideBase(step)
    , data_{data} {
  }

  [[nodiscard]] constexpr reference operator*() const noexcept {
    return *data_;
  }

  [[nodiscard]] constexpr pointer operator->() const noexcept {
    return data_;
  }

  constexpr SliceIterator& operator++() noexcept {
    data_ += StrideBase::Stride();
    return *this;
  }

  constexpr SliceIterator operator++(int) noexcept {
    SliceIterator tmp{*this};
    ++(*this);
    return tmp;
  }

  constexpr SliceIterator& operator--() noexcept {
    data_ -= StrideBase::Stride();
    return *this;
  }

  constexpr SliceIterator operator--(int) noexcept {
    SliceIterator tmp{*this};
    --*this;
    return tmp;
  }

  constexpr SliceIterator& operator+=(const difference_type offset) noexcept {
    data_ += offset * StrideBase::Stride();
    return *this;
  }

  constexpr SliceIterator& operator-=(const difference_type offset) noexcept {
    data_ -= offset * StrideBase::Stride();
    return *this;
  }

  [[nodiscard]] constexpr SliceIterator operator+(const difference_type offset) const noexcept {
    SliceIterator tmp{*this};
    tmp += offset;
    return tmp;
  }

  friend constexpr SliceIterator operator+(const difference_type offset, SliceIterator iter) noexcept {
    return iter + offset;
  }

  constexpr reference operator[](const difference_type offset) const noexcept {
    return *(data_ + offset);
  }

  [[nodiscard]] constexpr SliceIterator operator-(const difference_type offset) const noexcept {
    SliceIterator tmp{*this};
    tmp -= offset;
    return tmp;
  }

  [[nodiscard]] constexpr std::ptrdiff_t operator-(const SliceIterator other) const noexcept {
    return (data_ - other.data_) / StrideBase::Stride();
  }

  [[nodiscard]] constexpr bool operator==(const SliceIterator& rhs) const noexcept {
    return data_ == rhs.data_;
  }
  
  [[nodiscard]] constexpr std::strong_ordering operator<=>(const SliceIterator& rhs) const noexcept {
    return data_ <=> rhs.data_;
  }

  pointer data_ = nullptr;
};

template
  < class T
  , std::size_t extent = dynamic_extent
  , std::ptrdiff_t stride = 1
  >
class Slice
  : private detail::SizeBase<extent>
  , private detail::StrideBase<stride> { 
 private:
  using StrideBase = detail::StrideBase<stride>;
  using SizeBase = detail::SizeBase<extent>;
 
 public:
  using element_type     = T;
  using value_type       = std::remove_cv_t<T>;
  using size_type        = std::size_t;
  using difference_type  = std::ptrdiff_t;
  using pointer          = T*;
  using const_pointer    = const T*;
  using reference        = T&;
  using const_reference  = const T&;
  using iterator         = SliceIterator<T, stride>;
  using reverse_iterator = std::reverse_iterator<iterator>;

  constexpr Slice() noexcept
    : SizeBase(0)
    , StrideBase(1)
    , data_{nullptr} {
  }
  
  constexpr Slice(const Slice& slice) = default;
  constexpr Slice(Slice&& slice) = default;

  constexpr Slice& operator=(const Slice& slice) = default;
  constexpr Slice& operator=(Slice&& slice) = default;

  template <std::contiguous_iterator It>
  Slice(It first, std::size_t count, std::ptrdiff_t skip = (stride != dynamic_stride ? stride : 1))
    : SizeBase(count)
    , StrideBase(skip)
    , data_{std::to_address(first)} {
  }

  template <typename Iter>
  constexpr Slice(Iter first, Iter last, std::ptrdiff_t skip = (stride != dynamic_stride ? stride : 1))
    : SizeBase(std::distance(first, last))
    , StrideBase(skip)
    , data_{std::to_address(first)} {
  }

  template <typename TArr, std::size_t array_size>
  constexpr Slice(std::array<TArr, array_size>& array) noexcept
    : SizeBase(array_size)
    , StrideBase(1)
    , data_{std::data(array)} {
  }

  template <typename TArr, std::size_t array_size>
  constexpr Slice(const std::array<TArr, array_size>& array) noexcept
    : SizeBase(array_size)
    , StrideBase(1)
    , data_{std::data(array)} {
  }

  template <typename Range>
  constexpr Slice(Range&& range) requires 
    (requires(Range&& range) { range.Stride(); range.Size(); range.Data(); })
    : SizeBase(range.Size())
    , StrideBase(range.Stride())
    , data_{const_cast<T*>(range.Data())} {
  }

  template<class Range>
  constexpr Slice(Range&& range) requires (requires(Range&& range) { range.Stride(); } == false)
    : Slice{std::ranges::begin(range), std::ranges::end(range)} {
  }
  
  constexpr auto Size() const noexcept {
    return SizeBase::Size();
  }

  constexpr auto Stride() const noexcept {
    return StrideBase::Stride();
  }

  constexpr auto Data() const noexcept {
    return data_;
  }

  constexpr bool Empty() const noexcept {
    return this->Size() == 0;
  }

  constexpr reference operator[](std::size_t index) const noexcept {
    assert(index < this->Size());
    return data_[index * this->Stride()];
  }

  [[nodiscard]] constexpr iterator begin() const noexcept {
    return {Data(), Stride()};
  }

  [[nodiscard]] constexpr iterator end() const noexcept {
    return {Data() + Size() * Stride(), Stride()};
  }

  [[nodiscard]] constexpr reverse_iterator rbegin() const noexcept {
    return reverse_iterator{end()};
  }

  [[nodiscard]] constexpr reverse_iterator rend() const noexcept {
    return reverse_iterator{begin()};
  }

  Slice<T, dynamic_extent, stride>
  constexpr First(std::size_t count) const {
    assert(count < Size());
    return {data_, count, Stride()};
  };

  template <std::size_t count>
  constexpr Slice<T, count, stride>
  First() const {
    assert(count < Size());
    return {data_, count, Stride()};
  };

  constexpr Slice<T, dynamic_extent, stride>
  Last(std::size_t count) const {
    assert(count < Size());
    return {data_ + detail::EffectiveSize(Size() - count, Stride()), count, Stride()};
  };

  template <std::size_t count>
  constexpr Slice<T, count, stride>
  Last() const {
    assert(count < Size());
    return {data_ + detail::EffectiveSize(Size() - count, Stride()), count, Stride()};
  };

  constexpr Slice<T, dynamic_extent, stride>
  DropFirst(std::size_t count) const {
    assert(count < Size());
    return {data_ + detail::EffectiveSize(count, Stride()), Size() - count, Stride()};
  };

  template <std::size_t count>
  constexpr Slice<T, extent == dynamic_extent ? extent : extent - count, stride>
  DropFirst() const {
    assert(count < Size());
    return {data_ + detail::EffectiveSize(count, Stride()), Size() - count, Stride()};
  };

  constexpr Slice<T, dynamic_extent, stride>
  DropLast(std::size_t count) const {
    assert(count < Size());
    return {data_, Size() - count, Stride()};
  };

  template <std::size_t count>
  constexpr Slice<T, extent == dynamic_extent ? extent : extent - count, stride>
  DropLast() const {
    assert(count < Size());
    return {data_, Size() - count, Stride()};
  };

  constexpr Slice<T, dynamic_extent, dynamic_stride>
  Skip(std::ptrdiff_t skip) const {
    return {data_, Size() - (Size() / skip) * (skip - 1), Stride() * skip};
  };

  template <std::ptrdiff_t skip>
  constexpr Slice<T, extent == dynamic_extent ? extent : extent - (extent / skip) * (skip - 1), stride == dynamic_stride ? dynamic_stride : stride * skip>
  Skip() const {
    return {data_, Size() - (Size() / skip) * (skip - 1), Stride() * skip};
  };

  [[nodiscard]] constexpr bool operator==(const Slice& rhs) const noexcept {
    return data_ == rhs.data_ && Size() == rhs.Size() && Stride() == rhs.Stride();
  }
  
  [[nodiscard]] constexpr bool operator!=(const Slice& rhs) const noexcept {
    return !(*this == rhs);
  }
  
 private:
  T* data_;
};

template <typename Iter>
Slice(Iter, std::size_t, std::ptrdiff_t) -> Slice<std::iter_value_t<Iter>, dynamic_extent, dynamic_stride>;

template <typename Iter>
Slice(Iter, Iter, std::ptrdiff_t) -> Slice<std::iter_value_t<Iter>, dynamic_extent, dynamic_stride>;

template <typename Range>
Slice(Range&&) -> Slice<std::ranges::range_value_t<Range>>;

template <typename T, std::size_t size>
Slice(std::array<T, size>&) -> Slice<T, size>;
