#pragma once

/**
 * This file provides a basic wrapper for passing images throught the
 * application.
 *
 * All code is placed inside **img** namespace.
 */

#include "meta_types.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <memory>
#include <numeric>
#include <ranges>
#include <span>
#include <tuple>
#include <type_traits>
#include <vector>

namespace ssimp::img {

/**
 * Supported image element types enumeration
 */
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
/**
 * Type list of c++-types corresponding to the enumeration above
 */
using type_list = std::tuple<uint8_t,
                             uint16_t,
                             uint32_t,
                             uint64_t,
                             float,
                             double,
                             std::array<uint8_t, 3>,
                             std::array<uint8_t, 4>>;

/**
 * Convenience function for convering enum type to c++-type
 */
template <types type>
using enum_to_type = std::tuple_element_t<static_cast<int>(type), type_list>;

/**
 * Convenience function for convering c++-type to enum type
 */
template <typename T>
constexpr types type_to_enum =
    static_cast<types>(int(ssimp::mt::traits::tuple_type_idx_v<T, type_list>));

/**
 * Once again, now with type aliases to make the image manipulation as
 * convenient as possible
 */
using GRAY8 = enum_to_type<types::GRAY8>;
using GRAY16 = enum_to_type<types::GRAY16>;
using GRAY32 = enum_to_type<types::GRAY32>;
using GRAY64 = enum_to_type<types::GRAY64>;
using FLOAT = enum_to_type<types::FLOAT>;
using DOUBLE = enum_to_type<types::DOUBLE>;
using RGB8 = enum_to_type<types::RGB8>;
using RGBA8 = enum_to_type<types::RGBA8>;

/**
 * Concept to prevent instatiation of template swith unsupported type
 */
template <typename T>
concept ImgType =
    requires { requires ssimp::mt::concepts::TupleType<T, type_list>; };

} // namespace type_aliases

/**
 * Make type aliases visible in the **img** namespace (once again for
 * convenience :D)
 */
using namespace type_aliases;

template <typename T>
class ndImage;

/**
 * Base of all images.
 * It can be looked at as an type-erased wrapper for easier passing aroung.
 * It cannot be instatiated directly (there is not reasonable constructor),
 * to create an image, use **ndImage** below.
 *
 * Images can be passed by value, as it base contains all the attributes ndImage
 * needs.
 *
 * Copies via assignment operators are shallow. For deepcopy, use **copy**;
 *
 */
class ndImageBase {
  private:
	ndImageBase() = default;

  protected:
	ndImageBase(std::span<const std::size_t> dims,
	            std::size_t elem_size,
	            types type)
	    : _data(std::make_shared<std::vector<std::byte>>(
	          std::reduce(
	              dims.begin(), dims.end(), std::size_t(1), std::multiplies{}) *
	          elem_size)),
	      _dims(dims.begin(), dims.end()), _type(type) {}

  public:
	ndImageBase(ndImageBase&) = default;
	ndImageBase(ndImageBase&&) = default;
	ndImageBase& operator=(ndImageBase&) = default;
	ndImageBase& operator=(ndImageBase&&) = default;

	/**
	 * Get dimensions of image
	 */
	const std::vector<std::size_t>& dims() const { return _dims; }

	/**
	 * Original type of the image
	 */
	types type() const { return _type; }

	/**
	 * Return typed version of this image, that is: ndImage<T>.
	 * The image type must correspond to the original one.
	 */
	template <ImgType T>
	ndImage<T> as_typed() {
		return ndImage<T>(*this);
	}

	/**
	 * Deep copy of the image
	 */
	ndImageBase copy() const {
		ndImageBase cpy;
		cpy._type = _type;
		cpy._dims = _dims;
		cpy._data = std::make_shared<std::vector<std::byte>>(_data->begin(),
		                                                     _data->end());
		return cpy;
	}

  protected:
	std::shared_ptr<std::vector<std::byte>> _data;
	std::vector<std::size_t> _dims;
	types _type;
};

/**
 * Typed version of image with basic data access operators.
 * As of now, there is no image processing functionality included.
 *
 * Copies via assignment operators are shallow. For deepcopy, use **copy**;
 *
 */
template <typename T>
class ndImage : public ndImageBase {
  private:
	ndImage() = default;

  public:
	/**
	 * Construct new image with arbitrary number of dimensions.
	 * Dimensions are put directly to the constructor as *ints*.
	 *
	 * For example: ndImage<img::GRAY8>(1,2,3) will result in 8bit grayscale
	 * image with dimensions (1, 2, 3).
	 */
	template <typename... dims_t>
	    requires std::conjunction_v<std::is_same<int, dims_t>...>
	ndImage(dims_t... raw_dims)
	    : ndImage(std::array{std::size_t(raw_dims)...}) {
		assert(std::ranges::all_of(std::array{raw_dims...},
		                           [](auto x) { return x >= 0; }));
	}

	explicit ndImage(ndImageBase& base) : ndImageBase(base) {
		assert(base.type() == type_to_enum<T>);
	}

	/**
	 * Construct new image, obtain dimensions from continuous container.
	 */
	explicit ndImage(std::span<const std::size_t> sp)
	    : ndImageBase(sp, sizeof(T), type_to_enum<T>) {}

	std::span<T> span() {
		return {reinterpret_cast<T*>(_data->data()), _data->size() / sizeof(T)};
	}

	std::span<const T> span() const {
		return {reinterpret_cast<const T*>(_data->data()),
		        _data->size() / sizeof(T)};
	}

	/**
	 * Deep copy of the image
	 */
	ndImage copy() const {
		ndImage cpy(_dims);
		std::ranges::copy(*_data, cpy._data->begin());

		return cpy;
	}

	// ======== INDEXED ACCESS ==========
	/**
	 * Operator will change from () to [] when multidimensional subscript will
	 * be supported by all required compilers. Note that multidimensional
	 * subscribt is part of C++23.
	 *
	 * Currently, multidimensional subscript is implemented in GCC >= 12 and
	 * clang >= 15.
	 */

	T& operator()(std::size_t idx) {
		assert(idx < span().size());
		return span()[idx];
	}

	const T& operator()(std::size_t idx) const {
		assert(idx < span().size());
		return span()[idx];
	}

	template <typename... dims_t>
	    requires std::conjunction_v<std::is_same<int, dims_t>...>
	T& operator()(dims_t... coords) {
		return (*this)(std::array{std::size_t(coords)...});
	}

	template <typename... dims_t>
	    requires std::conjunction_v<std::is_same<int, dims_t>...>
	const T& operator()(dims_t... coords) const {
		return (*this)(std::array{std::size_t(coords)...});
	}

	T& operator()(std::span<const std::size_t> coords) {
		return (*this)(_get_flat_idx(coords));
	}
	const T& operator()(std::span<const std::size_t> coords) const {
		return (*this)(_get_flat_idx(coords));
	}

	// ======== ITERATOR ACCESS ==========
	auto begin() { return span().begin(); }
	auto begin() const { return span().begin(); }
	auto cbegin() const { return begin(); }
	auto rbegin() { return span().rbegin(); }
	auto rbegin() const { return span().rbegin(); }
	auto crbegin() const { return rbegin(); }

	auto end() { return span().end(); }
	auto end() const { return span().end(); }
	auto cend() const { return end(); }
	auto rend() { return span().rend(); }
	auto rend() const { return span().rend(); }
	auto crend() const { rend(); }

  private:
	/**
	 * Calculate flat index of the element.
	 * The index is calculated in such a way, that increasing first coordinate
	 * results in coalesced access.
	 */
	std::size_t _get_flat_idx(std::span<const std::size_t> sp) const {
		assert(sp.size() == _dims.size());

		std::size_t flat_idx = 0;
		std::size_t mult = 1;

		for (std::size_t i = 0; i < _dims.size(); ++i) {
			assert(sp[i] < _dims[i]);
			flat_idx += sp[i] * mult;
			mult *= _dims[i];
		}

		return flat_idx;
	}
};

} // namespace ssimp::img
