#pragma once

#include "../application/managers/options_manager.hpp"
#include "../application/meta_types.hpp"
#include "../application/nd_image.hpp"
#include <filesystem>
#include <vector>

namespace ssimp::formats {
class TestingSample {
  public:
	using supported_types = std::tuple<img::GRAY8>;
	constexpr static const char* name = "TestingSample";

	static std::vector<img::LocalizedImage>
	load_image(const std::filesystem::path&);

	template <typename T>
	    requires mt::traits::is_any_of_tuple_v<T,
	                                           TestingSample::supported_types>
	static void save_image(const img::ndImage<T>& img,
	                       const std::filesystem::path& path,
	                       const OptionsManager::options_t& options);
};
} // namespace ssimp::formats
