#include "api.hpp"
#include "managers/config_manager.hpp"
#include "managers/extension_manager.hpp"
#include "managers/format_manager.hpp"
#include "managers/options_manager.hpp"
#include "meta_types.hpp"
#include "nd_image.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int main() {
	ssimp::ConfigManager cfg;
	auto loaded = cfg.load_format("sample");
	std::cout << loaded << '\n';

	// boost::json::array root_tmp = loaded.as_array();
}
