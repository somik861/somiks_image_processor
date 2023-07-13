#include "api.hpp"
#include "managers/algorithm_manager.hpp"
#include "managers/config_manager.hpp"
#include "managers/extension_manager.hpp"
#include "managers/format_manager.hpp"
#include "managers/options_manager.hpp"
#include <algorithm>
#include <format>
#include <iostream>

namespace fs = std::filesystem;
using namespace std::literals;

namespace ssimp {
API::API()
    : _config_manager(std::make_unique<ConfigManager>()),
      _extension_manager(std::make_unique<ExtensionManager>()),
      _options_manager(std::make_unique<OptionsManager>()),
      _format_manager(std::make_unique<FormatManager>()),
      _algorithm_manager(std::make_unique<AlgorithmManager>()) {

	for (const std::string& format : _format_manager->registered()) {
		auto config = _config_manager->load_format(format).get_object();

		_extension_manager->load_from_json(
		    format, config.at("extensions").get_array(),
		    config.at("output_extension").get_string());

		_options_manager->load_from_json(
		    format + "_loading", config.at("loading_options").get_array());

		_options_manager->load_from_json(
		    format + "_saving", config.at("saving_options").get_array());
	}

	for (const std::string& algorithm : _algorithm_manager->registered()) {
		auto config = _config_manager->load_algorithm(algorithm).get_object();

		_options_manager->load_from_json(algorithm + "_algo",
		                                 config.at("options").get_array());
	}
}

std::string API::version() const { return "0.1"; }

std::vector<img::LocalizedImage>
API::load_image(const fs::path& path,
                const fs::path& rel_dir /* = "" */,
                const std::string& format /* = "" */,
                const option_types::options_t& options /* = {} */) const {
	fs::path img_prefix = "";
	if (!rel_dir.empty())
		img_prefix = fs::relative(path, rel_dir);

	auto load_with_format = [&](const std::string& format,
	                            const option_types::options_t& options) {
		if (!_options_manager->is_valid(format + "_loading", options))
			return std::optional<std::vector<img::LocalizedImage>>{};

		auto out = _format_manager->load_image(
		    path, format,
		    _options_manager->finalize_options(format + "_loading", options));
		if (out)
			for (auto& img : *out) {

				if (img.location.empty())
					img.location = path.stem();
				img.location = img_prefix / img.location;
			}
		return out;
	};

	if (!format.empty()) {
		auto out = load_with_format(format, options);
		if (out)
			return *out;
	} else {
		for (const auto& format :
		     _extension_manager->sorted_formats_by_priority(path)) {
			auto out = load_with_format(format, options);
			if (out)
				return *out;
		}
	}

	throw exceptions::Unsupported(
	    std::format("'{}' contains unsupported image", to_string(path)));
}

img::LocalizedImage
API::load_one(const fs::path& path,
              const fs::path& rel_dir /* = "" */,
              const std::string& format /* = "" */,
              const option_types::options_t& options /* = {} */) const {
	auto images = load_image(path, rel_dir, format, options);
	if (images.size() != 1)
		throw exceptions::IOError(
		    std::format("'{}' contains {} images, expected 1.", to_string(path),
		                images.size()));

	return images[0];
}

std::vector<std::vector<img::LocalizedImage>>
API::load_directory(const fs::path& dir,
                    bool recurse /* = false */,
                    const std::string& format /* = "" */,
                    const option_types::options_t& options /* = "" */) const {
	std::vector<std::vector<img::LocalizedImage>> out;
	_load_directory(out, dir, dir, recurse, format, options);
	return out;
}

void API::save_image(const std::vector<img::ndImageBase>& img,
                     const fs::path& path,
                     const std::string& format /* = "" */,
                     const option_types::options_t& options /* = {} */) const {
	for (const auto& f :
	     (format != "" ? std::vector{format}
	                   : _extension_manager->find_possible_formats(path))) {
		if (!img.empty() &&
		    !_format_manager->is_type_supported(format, img[0].type()))
			continue;

		if (!_format_manager->is_count_supported(format, img.size()))
			continue;

		if (std::ranges::any_of(img, [&](auto x) {
			    return !_format_manager->is_dims_supported(format, x.dims());
		    }))
			continue;

		if (!_options_manager->is_valid(format + "_saving", options))
			throw exceptions::Unsupported(std::format(
			    "Given options are not supported for format '{}'", format));

		_format_manager->save_image(
		    _extension_manager->with_output_extension(f, path), img, f,
		    _options_manager->finalize_options(format + "_saving", options));
		return;
	}

	throw exceptions::Unsupported(
	    std::format("No supported format found for given image and file: '{}'",
	                to_string(path.filename())));
}

void API::save_one(const img::ndImageBase& img,
                   const std::filesystem::path& path,
                   const std::string& format /* = "" */,
                   const option_types::options_t& options /* = {} */) const {
	save_image({img}, path, format, options);
}

ImageProperties
API::get_properties(const fs::path& path,
                    const option_types::options_t& options /* = {} */) const {
	for (const auto& format :
	     _extension_manager->sorted_formats_by_priority(path)) {
		if (!_options_manager->is_valid(format + "_loading", options))
			continue;
		auto out = _format_manager->get_image_information(
		    path, format,
		    _options_manager->finalize_options(format + "_loading", options));

		if (out)
			return *out;
	}

	throw exceptions::Unsupported(
	    std::format("'{}' contains unsupported image", to_string(path)));
}

std::vector<img::LocalizedImage>
API::apply(const std::vector<img::ndImageBase>& images,
           const std::string& algorithm,
           const option_types::options_t& options /* = {} */) const {

	if (!is_count_supported_algorithm(algorithm, images.size()))
		throw exceptions::Unsupported(
		    std::format("Algorithm '{}' does not support '{}' images",
		                algorithm, images.size()));

	for (const auto& img : images) {
		if (!is_dims_supported_algorithm(algorithm, img.dims()))
			throw exceptions::Unsupported(std::format(
			    "Algorithm '{}' does not support image of given dimensionality",
			    algorithm));
	}

	if (!images.empty() && is_same_dims_required_algorithm(algorithm)) {
		auto dims = images[0].dims();
		if (!std::ranges::all_of(images,
		                         [&](auto x) { return x.dims() == dims; }))
			throw exceptions::Unsupported(
			    std::format("Algorithm '{}' requires that images have the same "
			                "dimensionality.",
			                algorithm));
	}

	if (!_options_manager->is_valid(algorithm + "_algo", options))
		throw exceptions::Unsupported(std::format(
		    "Given options are not supported for algorithm '{}'", algorithm));

	return _algorithm_manager->apply(
	    images, algorithm,
	    _options_manager->finalize_options(algorithm + "_algo", options));
}

std::vector<img::LocalizedImage>
API::apply(const std::vector<img::LocalizedImage>& images,
           const std::string& algorithm,
           const option_types::options_t& options /* = {} */) const {
	std::vector<img::LocalizedImage> out;
	for (const auto& img : images) {
		auto res = apply({img.image}, algorithm, options);
		for (auto& res_img : res) {
			res_img.location = img.location.stem()
			                       .concat("_")
			                       .concat(res_img.location.c_str())
			                       .replace_extension(img.location.extension());
		}
		out.insert(out.end(), res.begin(), res.end());
	}

	return out;
}

std::set<std::string> API::supported_formats() const {
	auto formats = _format_manager->registered();
	return std::set(formats.begin(), formats.end());
}

std::set<std::string> API::supported_algorithms() const {
	auto algos = _algorithm_manager->registered();
	return std::set(algos.begin(), algos.end());
}

std::string API::predict_format(const fs::path& file) const {
	auto possibilites = _extension_manager->find_possible_formats(file);
	if (possibilites.empty())
		throw exceptions::Unsupported(
		    std::format("No supported format found for file: '{}'",
		                to_string(file.filename())));
	return possibilites[0];
}

bool API::is_count_supported_format(const std::string& format,
                                    std::size_t count) const {
	return _format_manager->is_count_supported(format, count);
}

bool API::is_dims_supported_format(const std::string& format,
                                   std::span<const std::size_t> dims) const {
	return _format_manager->is_dims_supported(format, dims);
}

bool API::is_same_dims_required_format(const std::string& format) const {
	return _format_manager->is_same_dims_required(format);
}

bool API::is_count_supported_algorithm(const std::string& algorithm,
                                       std::size_t count) const {
	return _algorithm_manager->is_count_supported(algorithm, count);
}

bool API::is_dims_supported_algorithm(const std::string& algorithm,
                                      std::span<const std::size_t> dims) const {
	return _algorithm_manager->is_dims_supported(algorithm, dims);
}

bool API::is_same_dims_required_algorithm(const std::string& algorithm) const {
	return _algorithm_manager->is_same_dims_required(algorithm);
}

bool API::is_type_supported_format(const std::string& format,
                                   img::elem_type type) const {
	return _format_manager->is_type_supported(format, type);
}

bool API::is_type_supported_algorithm(const std::string& algorithm,
                                      img::elem_type type) const {
	return _algorithm_manager->is_type_supported(algorithm, type);
}

std::set<img::elem_type>
API::supported_types_format(const std::string& format) const {
	auto types = _format_manager->supported_types(format);
	return std::set(types.begin(), types.end());
}

std::set<img::elem_type>
API::supported_types_algorithm(const std::string& algorithm) const {
	auto types = _algorithm_manager->supported_types(algorithm);
	return std::set(types.begin(), types.end());
}

const std::vector<ssimp::option_types::OptionConfig>&
API::loading_options_configuration(const std::string& format) const {
	return _options_manager->option_configs(format + "_loading");
}

const std::vector<ssimp::option_types::OptionConfig>&
API::saving_options_configuration(const std::string& format) const {
	return _options_manager->option_configs(format + "_saving");
}

const std::vector<ssimp::option_types::OptionConfig>&
API::algorithm_options_configuration(const std::string& algorithm) const {
	return _options_manager->option_configs(algorithm + "_algo");
}

std::vector<img::ndImageBase>
API::delocalize(const std::vector<img::LocalizedImage>& imgs) const {
	std::vector<img::ndImageBase> out;
	std::ranges::transform(imgs, std::back_inserter(out),
	                       &ssimp::img::LocalizedImage::image);

	return out;
}

API::~API() {}

void API::_load_directory(std::vector<std::vector<img::LocalizedImage>>& images,
                          const fs::path& base_dir,
                          const fs::path& curr_dir,
                          bool recurse,
                          const std::string& format,
                          const option_types::options_t& options) const {

	for (auto entry : fs::directory_iterator(curr_dir)) {
		if (entry.is_directory() && recurse)
			_load_directory(images, base_dir, entry.path(), recurse, format,
			                options);

		if (entry.is_regular_file())
			images.push_back(
			    load_image(entry.path(), base_dir, format, options));
	}
}

} // namespace ssimp
