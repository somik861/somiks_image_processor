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

std::vector<img::LocalizedImage>
API::load_image(const fs::path& path,
                const fs::path& rel_dir /* = "" */) const {
	fs::path img_prefix = "";
	if (!rel_dir.empty())
		img_prefix = fs::relative(path, rel_dir);

	for (const auto& format :
	     _extension_manager->sorted_formats_by_priority(path)) {
		auto out = _format_manager->load_image(path, format);
		if (!out.empty()) {
			for (auto& img : out)
				img.location = img_prefix / img.location;

			return out;
		}
	}

	throw exceptions::Unsupported(
	    std::format("'{}' contains unsupported image", to_string(path)));
}

img::LocalizedImage API::load_one(const fs::path& path,
                                  const fs::path& rel_dir /* = "" */) const {
	auto images = load_image(path, rel_dir);
	if (images.size() != 1)
		throw exceptions::IOError(
		    std::format("'{}' contains {} images, expected 1.", to_string(path),
		                images.size()));

	return images[0];
}

std::vector<img::LocalizedImage>
API::load_directory(const fs::path& dir, bool recurse /* = false */) const {
	std::vector<img::LocalizedImage> out;
	_load_directory(out, dir, dir, recurse);
	return out;
}

void API::save_image(const img::ndImageBase& img,
                     const fs::path& path,
                     const std::string& format /* = "" */,
                     const option_types::options_t& options /* = {} */) const {

	std::string final_format;
	if (!format.empty())
		final_format = format;
	else {
		auto formats = _extension_manager->find_possible_formats(path);
		if (format.empty())
			throw exceptions::Unsupported(
			    std::format("No supported format found for file: '{}'",
			                to_string(path.filename())));
		final_format = formats[0];
	}

	save_image({img, path.filename()}, path.parent_path(), final_format,
	           options);
}

void API::save_image(const img::LocalizedImage& img,
                     const std::filesystem::path& output_dir,
                     const std::string& format,
                     const option_types::options_t& options /* = {} */) const {

	if (!_format_manager->is_type_supported(format, img.image.type()))
		throw exceptions::Unsupported(
		    std::format("'{}' does not support type '{}'", format,
		                to_string(img.image.type())));
	if (!_options_manager->is_valid(format, options))
		throw exceptions::Unsupported(std::format(
		    "Given options are not supported for format '{}'", format));

	_format_manager->save_image(
	    output_dir,
	    {img.image,
	     _extension_manager->with_output_extension(format, img.location)},
	    format, _options_manager->finalize_options(format, options));
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

std::set<std::string> API::supported_formats() const {
	auto formats = _format_manager->registered_formats();
	return std::set(formats.begin(), formats.end());
}

std::set<std::string> API::supported_algorithms() const { return {}; }

std::string API::predict_format(const fs::path& file) const {
	auto possibilites = _extension_manager->find_possible_formats(file);
	if (possibilites.empty())
		throw exceptions::Unsupported(
		    std::format("No supported format found for file: '{}'",
		                to_string(file.filename())));
	return possibilites[0];
}

API::~API() {}

void API::_load_directory(std::vector<img::LocalizedImage>& images,
                          const fs::path& base_dir,
                          const fs::path& curr_dir,
                          bool recurse) const {

	for (auto entry : fs::directory_iterator(curr_dir)) {
		if (entry.is_directory() && recurse)
			_load_directory(images, base_dir, entry.path(), recurse);

		if (entry.is_regular_file()) {
			auto entry_images = load_image(entry.path(), base_dir);
			images.insert(images.end(), entry_images.begin(),
			              entry_images.end());
		}
	}
}

} // namespace ssimp
