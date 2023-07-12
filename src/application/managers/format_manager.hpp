#pragma once

#include "../../formats/jpeg.hpp"
#include "../../formats/png.hpp"
#include "../../formats/testing_sample.hpp"
#include "../nd_image.hpp"
#include "../utils.hpp"
#include "_algo_format_base.hpp"
#include "options_manager.hpp"
#include <filesystem>
#include <functional>
#include <optional>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace ssimp {
class FormatManager : public details::_AlgoFormatBase {
  private:
	using _registered_formats =
	    std::tuple</* formats::TestingSample, */ formats::JPEG, formats::PNG>;

  public:
	/**
	 * Modified constructor for format manager initialization.
	 */
	FormatManager();

	/**
	 * Open image at **path** using provided input **options**.
	 */
	std::optional<std::vector<img::LocalizedImage>>
	load_image(const std::filesystem::path& path,
	           const std::string& format,
	           const option_types::options_t& options) const;

	/**
	 * Save **image** to **path** using **format** with **options**.
	 */
	void save_image(const std::filesystem::path& path,
	                const std::vector<img::ndImageBase>& image,
	                const std::string& format,
	                const option_types::options_t& options) const;

	/**
	 * Get image information
	 */
	std::optional<ImageProperties>
	get_image_information(const std::filesystem::path& path,
	                      const std::string& format,
	                      const option_types::options_t options) const;

  private:
	using _loading_function_t =
	    std::function<std::optional<std::vector<img::LocalizedImage>>(
	        const std::filesystem::path&, const _options_t&)>;

	using _saving_function_t =
	    std::function<void(const std::vector<img::ndImageBase>&,
	                       const std::filesystem::path&,
	                       const _options_t&)>;

	using _info_function_t = std::function<std::optional<ImageProperties>(
	    const std::filesystem::path&, const option_types::options_t&)>;

	_funmap_t<_loading_function_t> _image_loaders;
	_funmap_t<_saving_function_t> _image_savers;
	_funmap_t<_info_function_t> _information_getters;
};
} // namespace ssimp
