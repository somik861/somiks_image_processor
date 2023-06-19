#include "api.hpp"
#include <boost/program_options.hpp>
#include <filesystem>
#include <iostream>
#include <string>

namespace po = boost::program_options;
namespace fs = std::filesystem;

// program options variables
fs::path _arg_input_path;
fs::path _arg_output_path;
std::string _arg_format = "";
fs::path _arg_options;
fs::path _arg_preset;
std::vector<std::string> _arg_algorithms;
fs::path _arg_algo_options;

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
	    ("preset", po::value<fs::path>(&_arg_preset), "path json preset file");

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

	if (vm.contains("preset"))
		return true;

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

int main(int argc, const char** argv) {
	ssimp::API api;
	try {
		if (!option_parser(argc, argv, api))
			return 0;

		// TODO: load options

		if (!_arg_directory_mode) {
			if (_arg_print_info) {
				std::cout << _arg_input_path << ":\n";
				std::cout << api.get_properties(_arg_input_path) << '\n';
				return 0;
			}

			auto images = api.load_image(_arg_input_path);
			if (images.size() == 1) {
				fs::create_directories(_arg_output_path.parent_path());
				api.save_image(images[0].image, _arg_output_path, _arg_format,
				               {});
			} else {
				fs::create_directories(_arg_output_path);
				for (const auto& img : images) {
					api.save_image(img, _arg_output_path, _arg_format, {});
				}
			}
		} else { // _arg_directory_node == true
			if (_arg_print_info) {
				print_info_dir(api, _arg_input_path, _arg_recurse);
				return 0;
			}

			auto images = api.load_directory(_arg_input_path, _arg_recurse);
			fs::create_directories(_arg_output_path);
			for (const auto& img : images) {
				api.save_image(img, _arg_output_path, _arg_format, {});
			}
		}

	} catch (std::exception& e) {
		std::cerr << "Error: " << e.what() << '\n';
		return 1;
	}
}
