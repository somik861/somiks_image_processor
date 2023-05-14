#pragma once
#include "meta_types.hpp"
#include <cassert>
#include <cstddef>
#include <memory>
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
using _get_img_type =
    std::tuple_element_t<static_cast<std::size_t>(type), type_list>;

using GRAY8 = _get_img_type<types::GRAY8>;
using GRAY16 = _get_img_type<types::GRAY16>;
using GRAY32 = _get_img_type<types::GRAY32>;
using GRAY64 = _get_img_type<types::GRAY64>;
using FLOAT = _get_img_type<types::FLOAT>;
using DOUBLE = _get_img_type<types::DOUBLE>;
using RGB8 = _get_img_type<types::RGB8>;
using RGBA8 = _get_img_type<types::RGBA8>;

template <typename T>
concept ImgType = requires { mt::concepts::TupleType<T, type_list>; };

} // namespace type_aliases

using namespace type_aliases;

template <typename T>
class ndImage;

class ndImageBase {
  public:
	ndImageBase() = delete;
	ndImageBase(const ndImageBase&) = default;
	ndImageBase(ndImageBase&&) = default;
	ndImageBase& operator=(const ndImageBase&) = default;
	ndImageBase& operator=(ndImageBase&&) = default;

	const std::vector<std::size_t>& dims() const { return _dims; }

	template <ImgType T>
	ndImage<T> as_typed() {
		assert((static_cast<int>(_type) == mt::traits::tuple_type_idx_v<T, type_list>));
		return ndImage<T>(*this);
	}

  protected:
	std::shared_ptr<std::vector<std::byte>> _data;
	std::vector<std::size_t> _dims;
	types _type;
};

} // namespace img
