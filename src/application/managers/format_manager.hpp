#pragma once

#include "../../formats/jpeg.hpp"
#include "../../formats/testing_sample.hpp"
#include "../nd_image.hpp"
#include "../utils.hpp"
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
	using _registered_formats =
	    std::tuple<formats::TestingSample, formats::JPEG>;

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
	 * Return true if type is supported.
	 */
	bool is_type_supported(const std::string& format,
	                       img::elem_type type) const;

	/**
	 * Return true if image count is supported
	 */
	bool is_count_supported(const std::string& format, std::size_t count) const;

	/**
	 * Return true if image dimensionality is supported
	 */
	bool is_dims_supported(const std::string& format,
	                       std::span<const std::size_t> dims) const;

	/**
	 * Get image information
	 */
	std::optional<ImageProperties>
	get_image_information(const std::filesystem::path& path,
	                      const std::string& format,
	                      const option_types::options_t options) const;

	/**
	 * Get names of registered formats
	 */
	std::unordered_set<std::string> registered_formats() const;

  private:
	using _options_t = option_types::options_t;
	template <typename fun_t>
	using _funmap_t = std::unordered_map<std::string, fun_t>;

	using _loading_function_t =
	    std::function<std::optional<std::vector<img::LocalizedImage>>(
	        const std::filesystem::path&, const _options_t&)>;

	using _saving_function_t =
	    std::function<void(const std::vector<img::ndImageBase>&,
	                       const std::filesystem::path&,
	                       const _options_t&)>;

	using _info_function_t = std::function<std::optional<ImageProperties>(
	    const std::filesystem::path&, const option_types::options_t&)>;

	using _count_verifier_t = std::function<bool(std::size_t)>;
	using _dims_verifier_t = std::function<bool(std::span<const std::size_t>)>;

	_funmap_t<_loading_function_t> _image_loaders;
	_funmap_t<_saving_function_t> _image_savers;
	_funmap_t<_info_function_t> _information_getters;
	_funmap_t<_count_verifier_t> _count_verifiers;
	_funmap_t<_dims_verifier_t> _dims_verifiers;

	std::unordered_map<std::string, std::unordered_set<img::elem_type>>
	    _format_supported_types;
};
} // namespace ssimp
