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

	template <typename T>
	class RowProxyIterator {
	  public:
		using element_type = T;
		using value_type = std::remove_cv_t<T>;
		using size_type = std::size_t;
		using pointer = T*;
		using const_pointer = const T*;
		using reference = T&;
		using const_reference = const T&;

		RowProxyIterator() = default;
		RowProxyIterator(T* data, std::size_t jump_size)
		    : _data(data), _jump_size(jump_size) {}
		constexpr auto operator<=>(const RowProxyIterator& o) const {
			return _data <=> o._data;
		}
		constexpr bool operator==(const RowProxyIterator& o) const {
			return o._data == _data;
		}
		constexpr std::ptrdiff_t operator-(const RowProxyIterator& o) const {
			return (o._data - _data) / _jump_size;
		}
		constexpr RowProxyIterator& operator++() {
			std::advance(_data, _jump_size);
			return *this;
		}
		constexpr RowProxyIterator operator++(int) {
			auto cpy = *this;
			++(*this);
			return cpy;
		}
		constexpr RowProxyIterator& operator--() {
			std::advance(_data, -_jump_size);
			return *this;
		}
		constexpr RowProxyIterator operator--(int) {
			auto cpy = *this;
			--(*this);
			return cpy;
		}
		constexpr T& operator*() const { return *_data; }
		constexpr T* operator->() const { return _data; }

		constexpr RowProxyIterator& operator+=(std::ptrdiff_t dist) {
			_data += _jump_size * dist;
			return *this;
		}

		constexpr RowProxyIterator& operator-=(std::ptrdiff_t dist) {
			_data -= _jump_size * dist;
			return *this;
		}

		constexpr RowProxyIterator operator+(std::ptrdiff_t dist) const {
			auto cpy = *this;
			cpy += dist;
			return cpy;
		}
		constexpr RowProxyIterator operator-(std::ptrdiff_t dist) const {
			auto cpy = *this;
			cpy -= dist;
			return cpy;
		}

		constexpr friend RowProxyIterator<T>
		operator+(std::ptrdiff_t dist, const RowProxyIterator<T>& o) {
			return o + dist;
		}
		constexpr friend RowProxyIterator<T>
		operator-(std::ptrdiff_t dist, const RowProxyIterator<T>& o) {
			return o - dist;
		}

		constexpr T& operator[](std::ptrdiff_t idx) const {
			return *(_data + _jump_size * idx);
		}

	  private:
		T* _data = nullptr;
		int _jump_size = 0;
	};

	template <typename img_t>
	class RowProxy {
	  public:
		using value_type = typename img_t::value_type;

		RowProxy(img_t* img,
		         std::size_t movable_dim,
		         std::vector<std::size_t> fixed_coords)
		    : _img(img), _movable_dim(movable_dim),
		      _coords(std::move(fixed_coords)) {
			assert(_movable_dim < img->dims().size());
			assert(_coords.size() + 1 == img->dims().size());
			_coords.insert(std::next(_coords.cbegin(), _movable_dim), 0);
		}
		value_type& operator[](std::size_t idx) {
			_coords[_movable_dim] = idx;
			return (*_img)(_coords);
		}

		value_type operator[](std::size_t idx) const {
			_coords[_movable_dim] = idx;
			return (*_img)(_coords);
		}

		std::size_t size() const { return _img->dims()[_movable_dim]; }

		auto begin() { return RowProxyIterator(&(*this)[0], _jump_size()); }
		auto begin() const {
			return RowProxyIterator(&(*this)[0], _jump_size());
		}
		auto cbegin() const { return begin(); }
		auto rbegin() {
			return RowProxyIterator(&(*this)[size() - 1], -_jump_size());
		}
		auto rbegin() const {
			return RowProxyIterator(&(*this)[size() - 1], -_jump_size());
		}
		auto crbegin() const { return rbegin(); }

		auto end() {
			return RowProxyIterator(
			    std::next(&(*this)[0], _jump_size() * size()), _jump_size());
		}
		auto end() const {
			return RowProxyIterator(
			    std::next(&(*this)[0], _jump_size() * size()), _jump_size());
		}
		auto cend() const { return end(); }
		auto rend() {
			return RowProxyIterator(std::next(&(*this)[0], -_jump_size()),
			                        -_jump_size());
		}
		auto rend() const {
			return RowProxyIterator(std::next(&(*this)[0], -_jump_size()),
			                        -_jump_size());
		}
		auto crend() const { return rend(); }

	  private:
		int _jump_size() const {
			return int(
			    std::reduce(_img->dims().begin(),
			                std::next(_img->dims().begin(), _movable_dim),
			                std::size_t{1}, std::multiplies{}));
		}

		img_t* _img;
		std::size_t _movable_dim;
		std::vector<std::size_t> _coords;
	};

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

	/**
	 * Apply function to all coordinates of the image.
	 * The function can either be T(T) or T(T, coords),
	 * where coords is a std::vector<std::size_t> showing
	 * current coords.
	 * The element is replaced with the one returned.
	 */
	template <typename func_t>
	void transform(func_t fun) {
		std::vector<std::size_t> current_coords(dims().size());
		auto increment_coords = [&]() {
			++current_coords[0];
			for (std::size_t i = 0; i < current_coords.size(); ++i) {
				if (current_coords[i] < dims()[i])
					break;
				current_coords[i] = 0;
				std::size_t next = i + 1;
				if (next < current_coords.size())
					++current_coords[next];
			}
		};

		do {
			if constexpr (std::is_invocable_r_v<T, func_t, T>)
				(*this)(current_coords) = fun((*this)(current_coords));

			else if constexpr (std::is_invocable_r_v<
			                       T, func_t, T,
			                       const std::vector<std::size_t>&>)
				(*this)(current_coords) =
				    fun((*this)(current_coords), current_coords);
			else
				static_assert(std::is_same_v<func_t, char> &&
				                  std::is_same_v<func_t, void>, // Always false
				              "Invalid function type");

			increment_coords();
		} while (std::ranges::any_of(current_coords, [](auto x) { return x; }));
	}

	/**
	 * Return (non-owning) proxy for row access.
	 *
	 * The row could be better understand as 1D-tile.
	 * The **movable_dim** defines along which dimension we want to move
	 * and **fixed_coords** specifies the indicies in other dimensions.
	 * Intuitively it can also be viewed as special case of slicing.
	 *
	 * From python.numpy: arr[0 , 1, :, 3, 4] is equivalent to
	 * **row**(2, {0, 1, 3, 4});
	 */
	auto row(std::size_t movable_dim,
	         const std::vector<std::size_t>& fixed_coords) {
		return RowProxy(this, movable_dim, fixed_coords);
	}

	auto row(std::size_t movable_dim,
	         const std::vector<std::size_t>& fixed_coords) const {
		return RowProxy(this, movable_dim, fixed_coords);
	}

	/**
	 * Apply function to all rows along movable dimension.
	 * The movable dimension has the same meaning as for **row(...)**.
	 *
	 * The function can either be void(RowProxy) or void(RowProxy,
	 * fixed_coords), where fixed_coords is std::vector<std::size_t> containing
	 * info about indicies to other dimensions. (the same as fixed_coords in
	 * **row(...)**)
	 *
	 * The function has to modify image through proxy, return value is ignored.
	 */
	template <typename func_t>
	void transform_rows(std::size_t movable_dim, func_t fun) {
		std::vector<std::size_t> end_ = dims();
		end_.erase(std::next(end_.begin(), movable_dim));
		std::vector<std::size_t> fixed_coords(dims().size() - 1);
		auto increment_coords = [&]() {
			++fixed_coords[0];
			for (std::size_t i = 0; i < fixed_coords.size(); ++i) {
				if (fixed_coords[i] < end_[i])
					break;
				fixed_coords[i] = 0;
				std::size_t next = i + 1;
				if (next < fixed_coords.size())
					++fixed_coords[next];
			}
		};

		do {
			if constexpr (std::is_invocable_v<func_t, RowProxy<ndImage<T>>&&>)
				fun(row(movable_dim, fixed_coords));

			else if constexpr (std::is_invocable_v<
			                       func_t, RowProxy<ndImage<T>>&&,
			                       const std::vector<std::size_t>&>)
				fun(row(movable_dim, fixed_coords), fixed_coords);
			else
				static_assert(std::is_same_v<func_t, char> &&
				                  std::is_same_v<func_t, void>, // Always false
				              "Invalid function type");

			increment_coords();
		} while (std::ranges::any_of(fixed_coords, [](auto x) { return x; }));
	}

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
