#include "api.hpp"
#include <algorithm>
#include <boost/json.hpp>
#include <boost/program_options.hpp>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>

#define print_debug(...)                                                       \
	if (_arg_debug)                                                            \
		std::cerr << "[DEBUG] " << std::format(__VA_ARGS__) << '\n';

namespace {
namespace po = boost::program_options;
namespace fs = std::filesystem;

// program options variables
fs::path _arg_input_path;
fs::path _arg_output_path;
std::string _arg_format = "";
fs::path _arg_loading_options = "";
fs::path _arg_saving_options = "";
fs::path _arg_preset = "";
std::vector<std::string> _arg_algorithms;
fs::path _arg_algo_options = "";
std::string _arg_saving_options_string = "";
std::string _arg_loading_options_string = "";
std::string _arg_algo_options_string = "";
std::string _arg_help_format = "";
std::string _arg_help_algo = "";

bool _arg_recurse = false;
bool _arg_directory_mode = false;
bool _arg_print_info = false;
bool _arg_debug = false;

boost::json::value _load_json_file(const std::filesystem::path& file) {
	std::ifstream f(file);
	return boost::json::parse(f);
}

void _load_preset() {
	auto preset = _load_json_file(_arg_preset).as_object();

	_arg_debug = preset.contains("debug") && preset.at("debug").as_bool();
	_arg_recurse = preset.contains("recurse") && preset.at("recurse").as_bool();
	_arg_print_info =
	    preset.contains("print_info") && preset.at("print_info").as_bool();

	if (preset.contains("algorithms"))
		std::ranges::transform(preset.at("algorithms").as_array(),
		                       std::back_inserter(_arg_algorithms), [](auto x) {
			                       return std::string(x.as_string());
		                       });

	_arg_format = preset.at("format").as_string();
}

void _print_options(
    const std::vector<ssimp::option_types::OptionConfig>& opts) {
	for (const auto& opt : opts)
		std::cout << opt << '\n';
}

bool option_parser(int argc, const char** argv, const ssimp::API& api) {
	po::positional_options_description positional;
	positional.add("input_path", 1).add("output_path", 1);

	po::options_description hidden("Hidden options");
	hidden.add_options() //
	    ("input_path", po::value<fs::path>(&_arg_input_path),
	     "input path") //
	    ("output_path", po::value<fs::path>(&_arg_output_path),
	     "output_path"); //

	po::options_description generic("Generic options");
	generic.add_options()                  //
	    ("help,h", "produce help message") //
	    ("version", "show version")        //
	    ("help_format", po::value(&_arg_help_format),
	     "show options config for format") //
	    ("help_algo", po::value(&_arg_help_algo),
	     "show options config for algorithm")                //
	    ("debug", "produce debug messages")                  //
	    ("print_info", "only print information about image") //
	    ("loading_options", po::value(&_arg_loading_options),
	     "path to json file containing options for loading files") //
	    ("loading_opt_string", po::value(&_arg_loading_options_string),
	     "json string (can be used instead of loading_options)") //
	    ("format,f", po::value(&_arg_format), "output format")   //
	    ("recurse,r", "Recurse into subdirectories")             //
	    ("saving_options", po::value(&_arg_saving_options),
	     "path to json file containing saving options for format") //
	    ("saving_opt_string", po::value(&_arg_saving_options_string),
	     "json string (can be used instead of saving_options)") //
	    ("algorithm,a", po::value(&_arg_algorithms),
	     "algorithms to use") //
	    ("algo_options", po::value(&_arg_algo_options),
	     "path to json file containing options for algorithms") //
	    ("algo_opt_string", po::value(&_arg_algo_options_string),
	     "json string (can be used instead of algo_options)") //
	    ("preset", po::value(&_arg_preset),
	     "path json preset file  !!!All other arguments will be ignored"
	     "!!!") //
	    ;

	po::options_description all_options("All options");
	all_options.add(hidden).add(generic);

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv)
	              .options(all_options)
	              .positional(positional)
	              .run(),
	          vm);

	if (vm.contains("help")) {
		std::cout << "Usage: ./ssimp input_path [output_path] "
		             "[--version]\n\t[--help] "
		             "[--help_format <string>] [--help_algo <string>]\n\t"
		             "[--recurse] [--preset "
		             "<preset.json>]\n\t[--loading_options "
		             "<lopt.json>] [--loading_opt_string "
		             "<string>]\n\t[--format <string> [--saving_options "
		             "<sopt.json>] [--saving_opt_string "
		             "<string>]]\n\t[{--algorithm <string>}... "
		             "[--algo_options <algo_options.json>] [--algo_opt_string "
		             "<string>]]\n\n";
		std::cout << generic << "\n";
		std::cout << "Supported formats:\n";
		for (const auto& f : api.supported_formats())
			std::cout << "\t" << f << '\n';
		std::cout << '\n';
		std::cout << "Supported algorithms:\n";
		for (const auto& a : api.supported_algorithms())
			std::cout << "\t" << a << '\n';
		std::cout << '\n';

		return false;
	}

	if (vm.contains("help_format")) {
		_arg_help_format = vm.at("help_format").as<std::string>();

		std::cout << _arg_help_format << " supported types: [ ";
		for (const auto& type : api.supported_types_format(_arg_help_format))
			std::cout << type << " ";
		std::cout << "]\n";

		std::cout << _arg_help_format << " loading options configuration:\n";
		_print_options(api.loading_options_configuration(_arg_help_format));
		std::cout << _arg_help_format << " saving options configuration:\n";
		_print_options(api.saving_options_configuration(_arg_help_format));
		return false;
	}

	if (vm.contains("help_algo")) {
		_arg_help_algo = vm.at("help_algo").as<std::string>();

		std::cout << _arg_help_algo << " supported types: [ ";
		for (const auto& type : api.supported_types_algorithm(_arg_help_algo))
			std::cout << type << " ";
		std::cout << "]\n";

		std::cout << _arg_help_algo << " options configuration:\n";
		_print_options(api.algorithm_options_configuration(_arg_help_algo));

		return false;
	}

	if (vm.contains("version")) {
		std::cout << "API version: " << api.version() << '\n';
		return false;
	}
	po::notify(vm);

	if (vm.contains("debug"))
		_arg_debug = true;

	if (!vm.contains("input_path"))
		throw std::runtime_error("Input path is required");

	if (!fs::exists(_arg_input_path))
		throw std::runtime_error("Input path does not exist");

	_arg_directory_mode = fs::is_directory(_arg_input_path);

	if (vm.contains("preset")) {
		_load_preset();
		return true;
	}

	if (!vm.contains("output_path") && !vm.contains("print_info"))
		throw std::runtime_error("Output path is required");

	if (!_arg_directory_mode && vm.contains("recurse"))
		throw std::runtime_error("recurse cannot be used if an "
		                         "input_path is not a directory.");

	_arg_recurse = vm.contains("recurse");
	if (vm.contains("print_info")) {
		_arg_print_info = true;
		return true;
	}

	if (_arg_directory_mode && !vm.contains("format"))
		throw std::runtime_error(
		    "--format is required when an input_path is a directory.");

	if (!vm.contains("format"))
		_arg_format = api.predict_format(_arg_output_path);

	if (vm.contains("options") && !vm.contains("format"))
		throw std::runtime_error("can not specify options without format");

	if (vm.contains("algo_options") && !vm.contains("algorithm"))
		throw std::runtime_error(
		    "can not specify algo_options without algorithm");

	if (vm.contains("loading_opt_string") && vm.contains("loading_options"))
		throw std::runtime_error("Only one of 'loading_options' and "
		                         "'loading_opt_string' can be used.");

	if (vm.contains("saving_opt_string") && vm.contains("saving_options"))
		throw std::runtime_error("Only one of 'saving_options' and "
		                         "'saving_opt_string' can be used.");

	if (vm.contains("algo_opt_string") && vm.contains("algo_options"))
		throw std::runtime_error("Only one of 'algo_options' and "
		                         "'algo_opt_string' can be used.");

	return true;
}

void print_info_dir(const ssimp::API& api,
                    const fs::path& curr_dir,
                    bool recurse,
                    const ssimp::option_types::options_t& loading_options) {
	for (auto& entry : fs::directory_iterator(curr_dir)) {
		if (entry.is_directory() && recurse)
			print_info_dir(api, entry.path(), recurse, loading_options);

		if (entry.is_regular_file()) {
			std::cout << entry << ":\n";
			std::cout << api.get_properties(entry.path()) << '\n';
		}
	}
}

ssimp::option_types::options_t
_load_json_options(const boost::json::object& data) {
	ssimp::option_types::options_t options;
	for (const auto& [key, value] : data) {
		ssimp::option_types::value_t opt_value;
		if (value.is_bool())
			opt_value = value.get_bool();

		else if (value.is_int64())
			opt_value = int32_t(value.get_int64());

		else if (value.is_double())
			opt_value = value.get_double();

		else if (value.is_string())
			opt_value = std::string(value.get_string());

		else
			throw std::runtime_error(std::format("unknown type of key: '{}'",
			                                     ssimp::to_string(key)));
		options[key] = std::move(opt_value);
	}

	return options;
}

std::unordered_map<std::string, ssimp::option_types::options_t>
_load_algo_json(const boost::json::object& options) {
	std::unordered_map<std::string, ssimp::option_types::options_t> out;

	for (const auto& [algo, options] :
	     _load_json_file(_arg_algo_options).as_object())
		out[algo] = _load_json_options(options.as_object());

	return out;
}

std::unordered_map<std::string, ssimp::option_types::options_t>
load_algo_options() {
	if (!_arg_preset.empty())
		return _load_algo_json(_load_json_file(_arg_preset)
		                           .as_object()
		                           .at("algo_options")
		                           .as_object());

	if (!_arg_algo_options.empty())
		return _load_algo_json(_load_json_file(_arg_algo_options).as_object());

	if (!_arg_algo_options_string.empty())
		return _load_algo_json(
		    boost::json::parse(_arg_algo_options_string).as_object());

	return {};
}

ssimp::option_types::options_t load_loading_options() {
	if (!_arg_preset.empty())
		_load_json_options(_load_json_file(_arg_preset)
		                       .as_object()
		                       .at("loading_options")
		                       .as_object());

	if (!_arg_loading_options.empty())
		return _load_json_options(
		    _load_json_file(_arg_loading_options).as_object());

	if (!_arg_loading_options_string.empty())
		return _load_json_options(
		    boost::json::parse(_arg_loading_options_string).as_object());

	return {};
}

ssimp::option_types::options_t load_saving_options() {
	if (!_arg_preset.empty())
		_load_json_options(_load_json_file(_arg_preset)
		                       .as_object()
		                       .at("saving_options")
		                       .as_object());

	if (!_arg_saving_options.empty())
		return _load_json_options(
		    _load_json_file(_arg_saving_options).as_object());

	if (!_arg_saving_options_string.empty())
		return _load_json_options(
		    boost::json::parse(_arg_saving_options_string).as_object());

	return {};
}

void save_image(const std::vector<ssimp::img::LocalizedImage>& images,
                const fs::path& path,
                const std::string& format,
                const ssimp::option_types::options_t& options,
                const ssimp::API& api) {
	if (api.is_count_supported_format(format, images.size()))
		api.save_image(api.delocalize(images), path, format, options);
	else if (api.is_count_supported_format(format, 1)) {
		fs::create_directories(path);
		for (const auto& img : images)
			api.save_one(img.image, path / img.location.filename(), format,
			             options);

	} else
		throw std::runtime_error(
		    "Did not find suitable way to save an image(s)");
}

std::vector<ssimp::img::LocalizedImage> _apply_algorithm(
    const std::vector<ssimp::img::LocalizedImage>& images,
    const std::string& algo,
    const std::unordered_map<std::string, ssimp::option_types::options_t>&
        algo_options,
    const ssimp::API& api) {
	ssimp::option_types::options_t options;

	if (algo_options.contains(algo))
		options = algo_options.at(algo);

	return api.apply(images, algo, options);
}

std::vector<ssimp::img::LocalizedImage> apply_algorithms(
    const std::vector<ssimp::img::LocalizedImage>& images,
    const std::unordered_map<std::string, ssimp::option_types::options_t>&
        algo_options,
    const ssimp::API& api) {

	auto out = images;
	for (const auto& algo : _arg_algorithms) {
		print_debug("applying {} on {} images", algo, out.size());
		out = _apply_algorithm(out, algo, algo_options, api);
		print_debug("got {} images", out.size());
	}
	return out;
}
} // namespace

int main(int argc, const char** argv) {
	ssimp::API api;

	try {
		if (!option_parser(argc, argv, api))
			return 0;

		print_debug("api loaded") print_debug("program options parsed");

		ssimp::option_types::options_t loading_options = load_loading_options();
		ssimp::option_types::options_t saving_options = load_saving_options();
		print_debug("format options loaded");

		std::unordered_map<std::string, ssimp::option_types::options_t>
		    algo_options = load_algo_options();
		print_debug("algo options loaded");

		if (!_arg_directory_mode) {
			print_debug("directory mode disabled");
			if (_arg_print_info) {
				print_debug("printing file information") std::cout
				    << _arg_input_path << ":\n";
				std::cout << api.get_properties(_arg_input_path,
				                                loading_options)
				          << '\n';
				print_debug("exiting ... (location 0)");
				return 0;
			}

			auto images =
			    api.load_image(_arg_input_path, "", "", loading_options);
			print_debug("{} loaded, got {} images",
			            ssimp::to_string(_arg_input_path), images.size());
			images = apply_algorithms(images, algo_options, api);
			print_debug("all algorithms applied, got {} images", images.size());

			fs::create_directories(_arg_output_path.parent_path());
			print_debug("{} created",
			            ssimp::to_string(_arg_output_path.parent_path()));
			print_debug("saving {} ...", ssimp::to_string(_arg_output_path));
			save_image(images, _arg_output_path, _arg_format, saving_options,
			           api);
			print_debug("saved");

		} else { // _arg_directory_node == true
			print_debug("directory mode enabled");
			if (_arg_print_info) {
				print_debug("printing directory info");
				print_info_dir(api, _arg_input_path, _arg_recurse,
				               loading_options);
				print_debug("exiting ... (location 1)");
				return 0;
			}

			auto all_images = api.load_directory(_arg_input_path, _arg_recurse,
			                                     "", loading_options);
			print_debug("images loaded, loaded {} files", all_images.size());
			fs::create_directories(_arg_output_path);
			for (auto& images : all_images) {
				if (images.empty())
					continue;
				print_debug("appling algorithms on {} images", images.size());
				images = apply_algorithms(images, algo_options, api);
				print_debug("all algorithms applied, got {} images",
				            images.size());

				fs::path out_path =
				    _arg_output_path / images[0].location.parent_path();
				print_debug("saving {} ...", ssimp::to_string(out_path));
				save_image(images, out_path, _arg_format, saving_options, api);
				print_debug("saved", ssimp::to_string(out_path));
			}
		}

	} catch (std::exception& e) {
		print_debug("printing error info");
		std::cerr << "Error: " << e.what() << '\n';
		print_debug("exiting ... (location 2)");
		return 1;
	}
	print_debug("exiting ... (location 3)");
}
