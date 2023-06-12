#pragma once

#include "nd_image.hpp"
#include "utils.hpp"
#include <filesystem>
#include <memory>
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

class API {
  public:
	/**
	 * Custom contructor to initalize everything
	 */
	API();

	/**
	 * Open file at **path**. If **rel_dir** is specified, the
	 * LocalizedImage.location path is set relative to **rel_dir**.
	 *
	 * Detect format from extension are try to open it.
	 * If failed, try every other supported format.
	 */
	std::vector<img::LocalizedImage>
	load_image(const std::filesystem::path& path,
	           const std::filesystem::path& rel_dir = "") const;

	/**
	 * Convenience function for loading file with single image inside.
	 * Look to **load_image(...)** for more info.
	 */
	img::LocalizedImage
	load_one(const std::filesystem::path& path,
	         const std::filesystem::path& rel_dir = "") const;

	/**
	 * Load all files in directory as images. For search in subdirectories,
	 * set **recurse** to *true*.
	 * Relative location of specific image to **dir** can be found via
	 * localizedImage.location.
	 */
	std::vector<img::LocalizedImage>
	load_directory(const std::filesystem::path& dir,
	               bool recurse = false) const;

	/**
	 * Save **img** to **path**.
	 * If **format** is not set, it is detected from **path**.
	 * Note that this still may change an extension if the **path** extension is
	 * not set as the output extension of found format.
	 */
	void save_image(const img::ndImageBase& img,
	                const std::filesystem::path& path,
	                const std::string& format = "",
	                const option_types::options_t& options = {}) const;

	/**
	 * Save image to **output_dir/img.location** with extension
	 * changed to respect **format** with **options**.
	 */
	void save_image(const img::LocalizedImage& img,
	                const std::filesystem::path& output_dir,
	                const std::string& format,
	                const option_types::options_t& options = {}) const;

	/**
	 * Get properties of image located at **path**.
	 */
	ImageProperties get_properties(const std::filesystem::path& path) const;

	/**
	 * Custom destructor to enable destruction of managers
	 */
	~API();

  private:
	void _load_directory(std::vector<img::LocalizedImage>& images,
	                     const std::filesystem::path& base_dir,
	                     const std::filesystem::path& curr_dir,
	                     bool recurse) const;

	std::unique_ptr<ConfigManager> _config_manager;
	std::unique_ptr<ExtensionManager> _extension_manager;
	std::unique_ptr<FormatManager> _format_manager;
	std::unique_ptr<OptionsManager> _options_manager;
};
} // namespace ssimp
