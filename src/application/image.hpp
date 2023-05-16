#pragma once
#include "meta_types.hpp"
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <numeric>
#include <tuple>
#include <type_traits>
#include <vector>

namespace img {

enum class types : int {
	GRAY8,
	GRAY16,
	GRAY32,
	GRAY64,
	FLOAT,
	DOUBLE,
	RGB8,
	RGBA8,
};

namespace type_aliases {
using type_list = std::tuple<uint8_t,
                             uint16_t,
                             uint32_t,
                             uint64_t,
                             float,
                             double,
                             std::array<uint8_t, 3>,
                             std::array<uint8_t, 4>>;

template <types type>
using enum_to_type = std::tuple_element_t<static_cast<int>(type), type_list>;

template <typename T>
constexpr types type_to_enum =
    static_cast<types>(int(mt::traits::tuple_type_idx_v<T, type_list>));

using GRAY8 = enum_to_type<types::GRAY8>;
using GRAY16 = enum_to_type<types::GRAY16>;
using GRAY32 = enum_to_type<types::GRAY32>;
using GRAY64 = enum_to_type<types::GRAY64>;
using FLOAT = enum_to_type<types::FLOAT>;
using DOUBLE = enum_to_type<types::DOUBLE>;
using RGB8 = enum_to_type<types::RGB8>;
using RGBA8 = enum_to_type<types::RGBA8>;

template <typename T>
concept ImgType = requires { mt::concepts::TupleType<T, type_list>; };

} // namespace type_aliases

using namespace type_aliases;

template <typename T>
class ndImage;

class ndImageBase {
  protected:
	ndImageBase(std::initializer_list<std::size_t> dims,
	            std::size_t data_size,
	            types type)
	    : _data(std::make_shared<std::vector<std::byte>>(data_size)),
	      _dims(dims), _type(type) {}

  public:
	ndImageBase() = delete;
	ndImageBase(const ndImageBase&) = default;
	ndImageBase(ndImageBase&&) = default;
	ndImageBase& operator=(const ndImageBase&) = default;
	ndImageBase& operator=(ndImageBase&&) = default;

	const std::vector<std::size_t>& dims() const { return _dims; }

	template <ImgType T>
	ndImage<T> as_typed() {
		assert(_type == type_to_enum<T>);
		return ndImage<T>(*this);
	}

  protected:
	std::shared_ptr<std::vector<std::byte>> _data;
	std::vector<std::size_t> _dims;
	types _type;
};

template <typename T>
class ndImage : public ndImageBase {
  public:
	template <typename... dims_t>
	    requires std::conjunction_v<std::is_same<std::size_t, dims_t>...>
	ndImage(dims_t... raw_dims) {
		std::initializer_list<std::size_t> dims = {raw_dims...};
		std::size_t data_size =
		    std::reduce(dims.begin(), dims.end(), 1, std::multiplies{}) *
		    sizeof(T);
		ndImageBase(dims, data_size, type_to_enum<T>);
	}
};

} // namespace img
