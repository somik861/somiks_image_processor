#pragma once

#include "nd_image.hpp"
#include "util_types.hpp"
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
	API();
	std::vector<img::LocalizedImage>
	load_image(const std::filesystem::path& path) const;

	void save_image(const img::LocalizedImage& img,
	                const std::filesystem::path& output_dir,
	                const option_types::options_t& options) const;

	ImageProperties get_properties(const std::filesystem::path& path) const;

  private:
	std::unique_ptr<ConfigManager> _config_manager;
	std::unique_ptr<ExtensionManager> _extension_manager;
	std::unique_ptr<FormatManager> _format_manager;
	std::unique_ptr<OptionsManager> _options_manager;
};
} // namespace ssimp
