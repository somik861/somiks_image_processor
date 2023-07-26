#pragma once

#include "common.hpp"

namespace ssimp::formats {
class PNG {
  public:
	using supported_types =
	    std::tuple<img::GRAY_8, img::GRAYA_8, img::RGB_8, img::RGBA_8>;
	constexpr static const char* name = "png";

	static bool image_count_supported(std::size_t count);
	static bool image_dims_supported(std::span<const std::size_t> dims);
	static constexpr bool same_dims_required() { return true; }

	static std::optional<std::vector<img::LocalizedImage>>
	load_image(const std::filesystem::path& path,
	           const option_types::options_t& options);

	template <typename T>
	    requires mt::traits::is_any_of_tuple_v<T, PNG::supported_types>
	static void save_image(const std::vector<img::ndImage<T>>& imgs,
	                       const std::filesystem::path& path,
	                       const option_types::options_t& options);

	static std::optional<ImageProperties>
	get_information(const std::filesystem::path& path,
	                const option_types::options_t& options);
};
} // namespace ssimp::formats
