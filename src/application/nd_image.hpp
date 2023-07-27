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
#include <complex>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <numeric>
#include <ostream>
#include <ranges>
#include <span>
#include <tuple>
#include <type_traits>
#include <vector>

namespace ssimp::img {

/**
 * Supported image element types enumeration
 */
enum class elem_type : int {
	GRAY_8,
	GRAYA_8,
	GRAY_16,
	GRAY_32,
	GRAY_64,
	FLOAT,
	DOUBLE,
	RGB_8,
	RGBA_8,
	COMPLEX_F,
	COMPLEX_D
};

inline std::ostream& operator<<(std::ostream& os, elem_type t) {
	os << std::array{"GRAY_8",  "GRAYA_8",   "GRAY_16",  "GRAY_32",
	                 "GRAY_64", "FLOAT",     "DOUBLE",   "RGB_8",
	                 "RGBA_8",  "COMPLEX_F", "COMPLEX_D"}[static_cast<int>(t)];
	return os;
}

namespace type_aliases {
/**
 * Type list of c++-types corresponding to the enumeration above
 */
using type_list = std::tuple<uint8_t,
                             std::array<uint8_t, 2>,
                             uint16_t,
                             uint32_t,
                             uint64_t,
                             float,
                             double,
                             std::array<uint8_t, 3>,
                             std::array<uint8_t, 4>,
                             std::complex<float>,
                             std::complex<double>>;

/**
 * Convenience function for convering enum type to c++-type
 */
template <elem_type type>
using enum_to_type = std::tuple_element_t<static_cast<int>(type), type_list>;

/**
 * Convenience function for convering c++-type to enum type
 */
template <typename T>
constexpr elem_type type_to_enum = static_cast<elem_type>(
    int(ssimp::mt::traits::tuple_type_idx_v<T, type_list>));

/**
 * Once again, now with type aliases to make the image manipulation as
 * convenient as possible
 */
using GRAY_8 = enum_to_type<elem_type::GRAY_8>;
using GRAYA_8 = enum_to_type<elem_type::GRAYA_8>;
using GRAY_16 = enum_to_type<elem_type::GRAY_16>;
using GRAY_32 = enum_to_type<elem_type::GRAY_32>;
using GRAY_64 = enum_to_type<elem_type::GRAY_64>;
using FLOAT = enum_to_type<elem_type::FLOAT>;
using DOUBLE = enum_to_type<elem_type::DOUBLE>;
using RGB_8 = enum_to_type<elem_type::RGB_8>;
using RGBA_8 = enum_to_type<elem_type::RGBA_8>;
using COMPLEX_F = enum_to_type<elem_type::COMPLEX_F>;
using COMPLEX_D = enum_to_type<elem_type::COMPLEX_D>;

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
	            elem_type type)
	    : _data(std::make_shared<std::vector<std::byte>>(
	          std::reduce(
	              dims.begin(), dims.end(), std::size_t(1), std::multiplies{}) *
	          elem_size)),
	      _dims(dims.begin(), dims.end()), _type(type) {}

  public:
	ndImageBase(const ndImageBase&) = default;
	ndImageBase(ndImageBase&&) = default;
	ndImageBase& operator=(const ndImageBase&) = default;
	ndImageBase& operator=(ndImageBase&&) = default;

	/**
	 * Get dimensions of image
	 */
	const std::vector<std::size_t>& dims() const { return _dims; }

	/**
	 * Original type of the image
	 */
	elem_type type() const { return _type; }

	/**
	 * Return typed version of this image, that is: ndImage<T>.
	 * The image type must correspond to the original one.
	 */
	template <ImgType T>
	ndImage<T> as_typed() {
		return ndImage<T>(*this);
	}

	template <ImgType T>
	const ndImage<T> as_typed() const {
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

	friend std::ostream& operator<<(std::ostream& os, const ndImageBase& img) {
		os << "Type: " << img.type() << '\n';
		os << "Dims: [ ";
		for (auto n : img._dims)
			os << n << " ";
		os << "]\n";

		return os;
	}

  protected:
	std::shared_ptr<std::vector<std::byte>> _data;
	std::vector<std::size_t> _dims;
	elem_type _type;
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
	using value_type = T;
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

	explicit ndImage(const ndImageBase& base) : ndImageBase(base) {
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

	// ======== DATA ACCESS ==========
	T* data() { return span().data(); }
	const T* data() const { return span().data(); }

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

/**
 * Wrapper containing image and its location information (mostly just name with
 * original extension or relative path from the input folder)
 */
class LocalizedImage {
  public:
	ndImageBase image;
	std::filesystem::path location;

	friend std::ostream& operator<<(std::ostream& os,
	                                const LocalizedImage& img) {
		os << "Location: " << img.location << '\n';
		os << img.image << '\n';

		return os;
	}
};

} // namespace ssimp::img
