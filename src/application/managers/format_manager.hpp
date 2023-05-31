#pragma once

#include "../../formats/testing_sample.hpp"
#include "../nd_image.hpp"
#include "options_manager.hpp"
#include <filesystem>
#include <functional>
#include <optional>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace ssimp {
class FormatManager {
  private:
	using _registered_formats = std::tuple<formats::TestingSample>;

  public:
	/**
	 * Modified constructor for format manager initialization.
	 */
	FormatManager();

	/**
	 * Detect automatically format and try to open the image.
	 */
	std::vector<img::LocalizedImage>
	load_image(const std::filesystem::path& path,
	           const std::string& format) const;

	/**
	 * Save **image** in **directory** using **image::location** as name.
	 */
	void save_image(const std::filesystem::path& directory,
	                const img::LocalizedImage& image,
	                const std::string& format,
	                const OptionsManager::options_t& options) const;

  private:
	std::unordered_map<std::string,
	                   std::function<std::vector<img::LocalizedImage>(
	                       const std::filesystem::path&)>>
	    _image_loaders;
	std::unordered_map<std::string,
	                   std::function<void(const img::ndImageBase&,
	                                      const std::filesystem::path&,
	                                      const OptionsManager::options_t&)>>
	    _image_savers;
};
} // namespace ssimp
