#include "config_manager.hpp"
#include "extension_matcher.hpp"
#include "image.hpp"
#include "meta_types.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int main() {
	ssimp::ConfigManager cfg;
	std::cout << cfg;
}
