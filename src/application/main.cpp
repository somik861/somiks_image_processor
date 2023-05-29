#include "extension_matcher.hpp"
#include "image.hpp"
#include "managers/config_manager.hpp"
#include "managers/options_manager.hpp"
#include "meta_types.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int main() {
	ssimp::ConfigManager cfg;
	auto loaded = cfg.load_format("sample");
	std::cout << loaded << '\n';

	// boost::json::array root_tmp = loaded.as_array();
}
