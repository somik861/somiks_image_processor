#pragma once

#include "common.hpp"

namespace ssimp::formats {
class TestingSample {
  public:
	using supported_types = std::tuple<img::GRAY8>;
	constexpr static const char* name = "testing_sample";

	static bool image_count_supported(std::size_t count);
	static bool image_dims_supported(std::span<const std::size_t> dims);

	static std::optional<std::vector<img::LocalizedImage>>
	load_image(const std::filesystem::path&,
	           const option_types::options_t& options);

	template <typename T>
	    requires mt::traits::is_any_of_tuple_v<T,
	                                           TestingSample::supported_types>
	static void save_image(const std::vector<img::ndImage<T>>& imgs,
	                       const std::filesystem::path& path,
	                       const option_types::options_t& options);

	static std::optional<ImageProperties>
	get_information(const std::filesystem::path& path,
	                const option_types::options_t& options);
};
} // namespace ssimp::formats
