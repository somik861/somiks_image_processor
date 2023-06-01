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
	                const option_types::options_t& options) const;

	/**
	 * Return true if type is supported.
	 */
	bool is_type_supported(const std::string& format,
	                       img::elem_type type) const;

	/**
	 * Get names of registered formats
	 */
	std::unordered_set<std::string> registered_formats() const;

  private:
	std::unordered_map<std::string,
	                   std::function<std::vector<img::LocalizedImage>(
	                       const std::filesystem::path&)>>
	    _image_loaders;
	std::unordered_map<std::string,
	                   std::function<void(const img::ndImageBase&,
	                                      const std::filesystem::path&,
	                                      const option_types::options_t&)>>
	    _image_savers;
	std::unordered_map<std::string, std::unordered_set<img::elem_type>>
	    _format_supported_types;
};
} // namespace ssimp
