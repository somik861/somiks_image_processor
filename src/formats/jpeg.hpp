#pragma once

#include "../application/managers/options_manager.hpp"
#include "../application/meta_types.hpp"
#include "../application/nd_image.hpp"
#include <filesystem>
#include <optional>
#include <vector>

namespace ssimp::formats {
class JPEG {
  public:
	using supported_types = std::tuple<img::GRAY8, img::RGB8>;
	constexpr static const char* name = "jpeg";

	static std::vector<img::LocalizedImage>
	load_image(const std::filesystem::path&);

	template <typename T>
	    requires mt::traits::is_any_of_tuple_v<T, JPEG::supported_types>
	static void save_image(const img::ndImage<T>& img,
	                       const std::filesystem::path& path,
	                       const option_types::options_t& options);

	static std::optional<ImageProperties>
	get_information(const std::filesystem::path& path);
};
} // namespace ssimp::formats
