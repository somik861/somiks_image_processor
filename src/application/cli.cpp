#include "api.hpp"
#include <algorithm>
#include <boost/json.hpp>
#include <boost/program_options.hpp>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>

namespace po = boost::program_options;
namespace fs = std::filesystem;

// program options variables
fs::path _arg_input_path;
fs::path _arg_output_path;
std::string _arg_format = "";
fs::path _arg_options = "";
fs::path _arg_preset = "";
std::vector<std::string> _arg_algorithms;
fs::path _arg_algo_options = "";

bool _arg_recurse = false;
bool _arg_directory_mode = false;
bool _arg_print_info = false;

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
	generic.add_options()                                                   //
	    ("help,h", "produce help message")                                  //
	    ("print_info", "only print information about image")                //
	    ("format,f", po::value<std::string>(&_arg_format), "output format") //
	    ("recurse,r", "Recurse into subdirectories")                        //
	    ("options", po::value<fs::path>(&_arg_options),
	     "path to json file containing options for format") //
	    ("algorithm,a", po::value<std::vector<std::string>>(&_arg_algorithms),
	     "algorithms to use") //
	    ("options", po::value<fs::path>(&_arg_algo_options),
	     "path to json file containing options for algorithms") //
	    // TODO implement later
	    ("preset", po::value<fs::path>(&_arg_preset),
	     "path json preset file <NOT IMPLEMENTED>");

	po::options_description all_options("All options");
	all_options.add(hidden).add(generic);

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv)
	              .options(all_options)
	              .positional(positional)
	              .run(),
	          vm);

	if (vm.contains("help")) {
		std::cout << "Usage: ./ssimp input_path output_path [--help] "
		             "[--recurse]\n\t[--preset "
		             "<preset.json>]\n\t[--format <string> [--options "
		             "<options.json>]]\n\t[{--algorithm <string>}... "
		             "[--algo_options <algo_options.json>]]\n\n";
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

	po::notify(vm);

	if (!vm.contains("input_path"))
		throw std::runtime_error("Input path is required");

	if (!vm.contains("output_path") && !vm.contains("print_info"))
		throw std::runtime_error("Output path is required");

	if (!fs::exists(_arg_input_path))
		throw std::runtime_error("Input path does not exist");

	_arg_directory_mode = fs::is_directory(_arg_input_path);

	if (!_arg_directory_mode && vm.contains("recurse"))
		throw std::runtime_error("--recurse cannot be used if an "
		                         "input_path is not a directory.");

	_arg_recurse = vm.contains("recurse");
	if (vm.contains("print_info")) {
		_arg_print_info = true;
		return true;
	}

	// TODO uncomment when preset implemented
	/*
	if (vm.contains("preset"))
	    return true;
	*/
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

	return true;
}

void print_info_dir(const ssimp::API& api,
                    const fs::path& curr_dir,
                    bool recurse) {
	for (auto& entry : fs::directory_iterator(curr_dir)) {
		if (entry.is_directory() && recurse)
			print_info_dir(api, entry.path(), recurse);

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
load_algo_options() {
	if (_arg_algo_options.empty())
		return {};

	std::unordered_map<std::string, ssimp::option_types::options_t> out;
	std::ifstream f(_arg_algo_options);
	boost::json::value data = boost::json::parse(f);
	for (const auto& [algo, options] : data.as_object())
		out[algo] = _load_json_options(options.as_object());

	return out;
}

ssimp::option_types::options_t load_format_options() {
	if (_arg_options.empty())
		return {};

	std::ifstream f(_arg_options);
	boost::json::value data = boost::json::parse(f);
	return _load_json_options(data.as_object());
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

int main(int argc, const char** argv) {
	ssimp::API api;
	try {
		if (!option_parser(argc, argv, api))
			return 0;

		ssimp::option_types::options_t format_options = load_format_options();

		std::unordered_map<std::string, ssimp::option_types::options_t>
		    algo_options = load_algo_options();

		if (!_arg_directory_mode) {
			if (_arg_print_info) {
				std::cout << _arg_input_path << ":\n";
				std::cout << api.get_properties(_arg_input_path) << '\n';
				return 0;
			}

			auto images = api.load_image(_arg_input_path);
			fs::create_directories(_arg_output_path.parent_path());
			save_image(images, _arg_output_path, _arg_format, format_options,
			           api);

		} else { // _arg_directory_node == true
			if (_arg_print_info) {
				print_info_dir(api, _arg_input_path, _arg_recurse);
				return 0;
			}

			auto all_images = api.load_directory(_arg_input_path, _arg_recurse);
			fs::create_directories(_arg_output_path);
			for (auto& images : all_images) {
				if (images.empty())
					continue;
				fs::path out_path =
				    _arg_output_path / images[0].location.parent_path();
				save_image(images, out_path, _arg_format, format_options, api);
			}
		}

	} catch (std::exception& e) {
		std::cerr << "Error: " << e.what() << '\n';
		return 1;
	}
}
