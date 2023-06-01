#include "api.hpp"
#include "managers/config_manager.hpp"
#include "managers/extension_manager.hpp"
#include "managers/format_manager.hpp"
#include "managers/options_manager.hpp"

namespace ssimp {
API::API()
    : _config_manager(std::make_unique<ConfigManager>()),
      _extension_manager(std::make_unique<ExtensionManager>()),
      _format_manager(std::make_unique<FormatManager>()),
      _options_manager(std::make_unique<OptionsManager>()) {

	for (const std::string& format : _format_manager->registered_formats()) {
		auto config = _config_manager->load_format(format).get_object();

		_extension_manager->load_from_json(
		    format, config.at("extensions").get_array(),
		    config.at("output_extension").get_string());

		_options_manager->load_from_json(format,
		                                 config.at("options").get_array());
	}
}

} // namespace ssimp
