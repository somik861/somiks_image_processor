#pragma once

#include "nd_image.hpp"
#include "utils.hpp"
#include <filesystem>
#include <memory>
#include <set>
#include <vector>

namespace ssimp {
/**
 * Forward declaration to prevent importing it with 3rd party libs.
 * This will ease-up use when the ssimp is used as a library, since it will not
 * need additional libraries to compile.
 */
class ConfigManager;
class ExtensionManager;
class FormatManager;
class OptionsManager;
class AlgorithmManager;

class API {
  public:
	/**
	 * Custom contructor to initalize everything
	 */
	API();

	std::string version() const;

	/**
	 * Open file at **path**. If **rel_dir** is specified, the
	 * LocalizedImage.location path is set relative to **rel_dir**.
	 *
	 * Detect format from extension are try to open it.
	 * If failed, try every other supported format.
	 */
	std::vector<img::LocalizedImage>
	load_image(const std::filesystem::path& path,
	           const std::filesystem::path& rel_dir = "",
	           const std::string& format = "",
	           const option_types::options_t& options = {}) const;

	/**
	 * Convenience function for loading file with single image inside.
	 * Look to **load_image(...)** for more info.
	 */
	img::LocalizedImage
	load_one(const std::filesystem::path& path,
	         const std::filesystem::path& rel_dir = "",
	         const std::string& format = "",
	         const option_types::options_t& options = {}) const;

	/**
	 * Load all files in directory as images. For search in subdirectories,
	 * set **recurse** to *true*.
	 * Relative location of specific image to **dir** can be found via
	 * localizedImage.location.
	 */
	std::vector<std::vector<img::LocalizedImage>>
	load_directory(const std::filesystem::path& dir,
	               bool recurse = false,
	               const std::string& format = "",
	               const option_types::options_t& options = {}) const;

	/**
	 * Save **img** to **path**.
	 * If **format** is not set, it is detected from **path**.
	 * Note that this still may change an extension if the **path** extension is
	 * not set as the output extension of found format.
	 */
	void save_image(const std::vector<img::ndImageBase>& img,
	                const std::filesystem::path& path,
	                const std::string& format = "",
	                const option_types::options_t& options = {}) const;

	/**
	 * Convenience function for saving file with single image inside
	 * Look to **save_image(...)** for more info
	 */
	void save_one(const img::ndImageBase& img,
	              const std::filesystem::path& path,
	              const std::string& format = "",
	              const option_types::options_t& options = {}) const;

	/**
	 * Get properties of image located at **path**.
	 */
	ImageProperties
	get_properties(const std::filesystem::path& path,
	               const option_types::options_t& options = {}) const;

	/**
	 * Apply **algorithm** on image(s).
	 */
	std::vector<img::LocalizedImage>
	apply(const std::vector<img::ndImageBase>& images,
	      const std::string& algorithm,
	      const option_types::options_t& options = {}) const;

	/**
	 * Apply **algorithm** on image(s) one by one and use location information
	 * to mark their source.
	 */
	std::vector<img::LocalizedImage>
	apply(const std::vector<img::LocalizedImage>& images,
	      const std::string& algorithm,
	      const option_types::options_t& options = {}) const;

	/*
	 * Get supported formats
	 */
	std::set<std::string> supported_formats() const;

	/*
	 * Get supported algorithms
	 */
	std::set<std::string> supported_algorithms() const;

	/**
	 * Get predicted format from extension
	 */
	std::string predict_format(const std::filesystem::path& file) const;

	/**
	 * Return true if image count is supported by **format**
	 */
	bool is_count_supported_format(const std::string& format,
	                               std::size_t count) const;

	/**
	 * Return true if image dimensionality is supported by **format**
	 */
	bool is_dims_supported_format(const std::string& format,
	                              std::span<const std::size_t> dims) const;

	/**
	 * Return true if same dimensionality of images is required by **format**
	 */
	bool is_same_dims_required_format(const std::string& format) const;

	/**
	 * Return true if image count is supported by **algorithm**
	 */
	bool is_count_supported_algorithm(const std::string& algorithm,
	                                  std::size_t count) const;

	/**
	 * Return true if image dimensionality is supported by **algorithm**
	 */
	bool is_dims_supported_algorithm(const std::string& algorithm,
	                                 std::span<const std::size_t> dims) const;

	/**
	 * Return true if same dimensionality of images is required by **algorithm**
	 */
	bool is_same_dims_required_algorithm(const std::string& algorithm) const;

	/**
	 * Return true if the image type is supported by **format**
	 */
	bool is_type_supported_format(const std::string& format,
	                              img::elem_type type) const;

	/**
	 * Return true if the image type is supported by **algorithm**
	 */
	bool is_type_supported_algorithm(const std::string& algorithm,
	                                 img::elem_type type) const;

	/**
	 * Return set of image types supported by **format**
	 */
	std::set<img::elem_type>
	supported_types_format(const std::string& format) const;

	/**
	 * Return set of image types supported by **format**
	 */
	std::set<img::elem_type>
	supported_types_algorithm(const std::string& algorithm) const;

	/**
	 * Get loading option configuration of **format**
	 */
	const std::vector<ssimp::option_types::OptionConfig>&
	loading_options_configuration(const std::string& format) const;

	/**
	 * Get saving option configuration of **format**
	 */
	const std::vector<ssimp::option_types::OptionConfig>&
	saving_options_configuration(const std::string& format) const;

	/**
	 * Get option configuration of **algorithm**
	 */
	const std::vector<ssimp::option_types::OptionConfig>&
	algorithm_options_configuration(const std::string& algorithm) const;

	/**
	 * Transform Localized images back to ndImageBase
	 */
	std::vector<img::ndImageBase>
	delocalize(const std::vector<img::LocalizedImage>& imgs) const;

	/**
	 * Return name of file with corrected extension (if neccessary)
	 */
	std::filesystem::path
	with_correct_extension(const std::string& format,
	                       const std::filesystem::path& file) const;

	/**
	 * Custom destructor to enable destruction of managers
	 */
	~API();

  private:
	void _load_directory(std::vector<std::vector<img::LocalizedImage>>& images,
	                     const std::filesystem::path& base_dir,
	                     const std::filesystem::path& curr_dir,
	                     bool recurse,
	                     const std::string& format,
	                     const option_types::options_t& options) const;

	void _check_format_validity(const std::string& format) const;
	void _check_algorithm_validity(const std::string& algorithm) const;

	std::unique_ptr<ConfigManager> _config_manager;
	std::unique_ptr<ExtensionManager> _extension_manager;
	std::unique_ptr<OptionsManager> _options_manager;
	std::unique_ptr<FormatManager> _format_manager;
	std::unique_ptr<AlgorithmManager> _algorithm_manager;
};
} // namespace ssimp
