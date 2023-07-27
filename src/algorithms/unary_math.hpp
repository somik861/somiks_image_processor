#pragma once

#include "common.hpp"

namespace ssimp::algorithms {
class UnaryMath {
  public:
	using supported_types = std::tuple<img::GRAY_8,
	                                   img::GRAY_16,
	                                   img::GRAY_32,
	                                   img::GRAY_64,
	                                   img::GRAYA_8,
	                                   img::FLOAT,
	                                   img::DOUBLE,
	                                   img::COMPLEX_F,
	                                   img::COMPLEX_D>;
	constexpr static const char* name = "unary_math";

	static bool image_count_supported(std::size_t count);
	static bool image_dims_supported(std::span<const std::size_t> dims);
	static bool same_dims_required();

	template <typename T>
	    requires mt::traits::is_any_of_tuple_v<T, UnaryMath::supported_types>
	static std::vector<img::LocalizedImage>
	apply(const std::vector<img::ndImage<T>>& imgs,
	      const option_types::options_t& options);
};

} // namespace ssimp::algorithms
