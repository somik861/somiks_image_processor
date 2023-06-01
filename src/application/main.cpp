#include "api.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int main() {

	try {

		ssimp::API api;

		std::cout << api.get_properties("lenna.jpg");

	} catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
	}
}
