#include "api.hpp"
#include "managers/config_manager.hpp"
#include "managers/extension_manager.hpp"
#include "managers/format_manager.hpp"
#include "managers/options_manager.hpp"
#include <format>
#include <iostream>

namespace fs = std::filesystem;

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

std::vector<img::LocalizedImage> API::load_image(const fs::path& path) const {
	for (const auto& format :
	     _extension_manager->sorted_formats_by_priority(path)) {
		auto out = _format_manager->load_image(path, format);
		if (!out.empty())
			return out;
	}

	throw exceptions::Unsupported(
	    std::format("'{}' contains unsupported image", to_string(path)));
}

void API::save_image(img::LocalizedImage img,
                     const std::filesystem::path& output_dir,
                     const std::string& format,
                     const option_types::options_t& options) const {

	if (!_format_manager->is_type_supported(format, img.image.type()))
		throw exceptions::Unsupported(
		    std::format("'{}' does not support type '{}'", format,
		                to_string(img.image.type())));
	if (!_options_manager->is_valid(format, options))
		throw exceptions::Unsupported(std::format(
		    "Given options are not supported for format '{}'", format));

	img.location =
	    _extension_manager->with_output_extension(format, img.location);

	_format_manager->save_image(
	    output_dir, img, format,
	    _options_manager->finalize_options(format, options));
}

ImageProperties API::get_properties(const fs::path& path) const {
	for (const auto& format :
	     _extension_manager->sorted_formats_by_priority(path)) {
		auto out = _format_manager->get_image_information(path, format);

		if (out)
			return *out;
	}

	throw exceptions::Unsupported(
	    std::format("'{}' contains unsupported image", to_string(path)));
}

API::~API() {}

} // namespace ssimp
