#pragma once

#include <cassert>
#include <functional>
#include <type_traits>
#include <utility>
#include <variant>

namespace openmedia {

template<class T>
struct OkTag {
  T value;
};

template<class E>
struct ErrTag {
  E value;
};

template<class T>
auto Ok(T&& value) -> OkTag<T&&> {
  return {std::forward<T>(value)};
}

template<class E>
auto Err(E&& error) -> ErrTag<E&&> {
  return {std::forward<E>(error)};
}

template<class T, class E>
class Result {
  template<class K>
  struct StorageType {
    using type = K;
  };

  template<class K>
  struct StorageType<K&> {
    using type = std::reference_wrapper<std::remove_reference_t<K>>;
  };

  template<class K>
  using storage_t = typename StorageType<K>::type;

  using ok_storage_t = storage_t<T>;
  using err_storage_t = storage_t<E>;

  std::variant<ok_storage_t, err_storage_t> storage_;

  template<class U>
  static auto wrapValue(U&& value) -> ok_storage_t {
    if constexpr (std::is_reference_v<T>) {
      return std::ref(value);
    } else {
      return ok_storage_t(std::forward<U>(value));
    }
  }

  template<class U>
  static auto wrapError(U&& error) -> err_storage_t {
    if constexpr (std::is_reference_v<E>) {
      return std::ref(error);
    } else {
      return err_storage_t(std::forward<U>(error));
    }
  }

  explicit Result(std::in_place_index_t<0>, ok_storage_t v)
      : storage_(std::in_place_index<0>, std::move(v)) {}

  explicit Result(std::in_place_index_t<1>, err_storage_t e)
      : storage_(std::in_place_index<1>, std::move(e)) {}

public:
  using ok_type = T;
  using err_type = E;

  Result() = delete;
  Result(const Result&) = default;
  Result(Result&&) noexcept = default;
  auto operator=(const Result&) -> Result& = default;
  auto operator=(Result&&) noexcept -> Result& = default;

  static auto makeOk(T value) -> Result {
    return Result(std::in_place_index<0>, wrapValue(std::forward<T>(value)));
  }

  static auto makeErr(E error) -> Result {
    return Result(std::in_place_index<1>, wrapError(std::forward<E>(error)));
  }

  template<class U>
  Result(OkTag<U> tag)
      : storage_(std::in_place_index<0>, wrapValue(std::forward<U>(tag.value))) {}

  template<class U>
  Result(ErrTag<U> tag)
      : storage_(std::in_place_index<1>, wrapError(std::forward<U>(tag.value))) {}

  auto isOk() const noexcept -> bool { return storage_.index() == 0; }
  auto isErr() const noexcept -> bool { return storage_.index() == 1; }

  explicit operator bool() const noexcept { return isOk(); }

  auto unwrap() & -> T& {
    assert(isOk() && "Result::unwrap() called on Err");
    if constexpr (std::is_reference_v<T>) {
      return std::get<0>(storage_).get();
    } else {
      return std::get<0>(storage_);
    }
  }

  auto unwrap() const& -> const T& {
    assert(isOk() && "Result::unwrap() called on Err");
    if constexpr (std::is_reference_v<T>) {
      return std::get<0>(storage_).get();
    } else {
      return std::get<0>(storage_);
    }
  }

  auto unwrap() && -> T {
    assert(isOk() && "Result::unwrap() called on Err");
    if constexpr (std::is_reference_v<T>) {
      return std::get<0>(storage_).get();
    } else {
      return std::move(std::get<0>(storage_));
    }
  }

  auto unwrapErr() & -> E& {
    assert(isErr() && "Result::unwrapErr() called on Ok");
    if constexpr (std::is_reference_v<E>) {
      return std::get<1>(storage_).get();
    } else {
      return std::get<1>(storage_);
    }
  }

  auto unwrapErr() const& -> const E& {
    assert(isErr() && "Result::unwrapErr() called on Ok");
    if constexpr (std::is_reference_v<E>) {
      return std::get<1>(storage_).get();
    } else {
      return std::get<1>(storage_);
    }
  }

  auto unwrapErr() && -> E {
    assert(isErr() && "Result::unwrapErr() called on Ok");
    if constexpr (std::is_reference_v<E>) {
      return std::get<1>(storage_).get();
    } else {
      return std::move(std::get<1>(storage_));
    }
  }

  template<class U>
  auto unwrapOr(U&& fallback) const& -> T {
    return isOk() ? unwrap() : static_cast<T>(std::forward<U>(fallback));
  }

  template<class U>
  auto unwrapOr(U&& fallback) && -> T {
    return isOk() ? std::move(*this).unwrap() : static_cast<T>(std::forward<U>(fallback));
  }

  template<class F>
  auto map(F&& fn) & {
    using U = std::invoke_result_t<F, T&>;
    using ResultU = Result<U, E>;
    if (isOk()) return ResultU::makeOk(std::invoke(std::forward<F>(fn), unwrap()));
    return ResultU::makeErr(unwrapErr());
  }

  template<class F>
  auto map(F&& fn) const& {
    using U = std::invoke_result_t<F, const T&>;
    using ResultU = Result<U, E>;
    if (isOk()) return ResultU::makeOk(std::invoke(std::forward<F>(fn), unwrap()));
    return ResultU::makeErr(unwrapErr());
  }

  template<class F>
  auto map(F&& fn) && {
    using U = std::invoke_result_t<F, T>;
    using ResultU = Result<U, E>;
    if (isOk()) return ResultU::makeOk(std::invoke(std::forward<F>(fn), std::move(*this).unwrap()));
    return ResultU::makeErr(std::move(*this).unwrapErr());
  }

  template<class F>
  auto mapErr(F&& fn) & {
    using G = std::invoke_result_t<F, E&>;
    using ResultG = Result<T, G>;
    if (isErr()) return ResultG::makeErr(std::invoke(std::forward<F>(fn), unwrapErr()));
    return ResultG::makeOk(unwrap());
  }

  template<class F>
  auto mapErr(F&& fn) && {
    using G = std::invoke_result_t<F, E>;
    using ResultG = Result<T, G>;
    if (isErr()) return ResultG::makeErr(std::invoke(std::forward<F>(fn), std::move(*this).unwrapErr()));
    return ResultG::makeOk(std::move(*this).unwrap());
  }

  template<class F>
  auto andThen(F&& fn) & {
    using Ret = std::invoke_result_t<F, T&>;
    if (isOk()) return std::invoke(std::forward<F>(fn), unwrap());
    return Ret::makeErr(unwrapErr());
  }

  template<class F>
  auto andThen(F&& fn) && {
    using Ret = std::invoke_result_t<F, T>;
    if (isOk()) return std::invoke(std::forward<F>(fn), std::move(*this).unwrap());
    return Ret::makeErr(std::move(*this).unwrapErr());
  }
};

template<class T, class E>
Result(OkTag<T>, ErrTag<E>) -> Result<T, E>;

} // namespace openmedia
