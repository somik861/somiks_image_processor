#pragma once

#include "common.hpp"

namespace ssimp::algorithms {
class ToGray8 {
  public:
	using supported_types = std::tuple<img::GRAY8,
	                                   img::GRAY8A,
	                                   img::GRAY16,
	                                   img::GRAY32,
	                                   img::GRAY64,
	                                   img::RGB8,
	                                   img::RGBA8,
	                                   img::FLOAT,
	                                   img::DOUBLE>;
	constexpr static const char* name = "to_gray8";

	static bool image_count_supported(std::size_t count);
	static bool image_dims_supported(std::span<const std::size_t> dims);
	static bool same_dims_required();

	template <typename T>
	    requires mt::traits::is_any_of_tuple_v<T, ToGray8::supported_types>
	static std::vector<img::LocalizedImage>
	apply(const std::vector<img::ndImage<T>>& imgs,
	      const option_types::options_t& options);
};

} // namespace ssimp::algorithms
