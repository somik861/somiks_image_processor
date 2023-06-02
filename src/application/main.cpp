#include "api.hpp"
#include <filesystem>
#include <format>
#include <iostream>

namespace fs = std::filesystem;

int main() {

	try {

		ssimp::API api;

		std::cout << api.get_properties("lenna.jpg");

		auto img = api.load_image("lenna.jpg");
		auto typed = img[0].image.as_typed<ssimp::img::RGB8>();

		auto [r, g, b] = typed(1, 0);
		std::cout << std::format("r: {} g: {} b: {}", r, g, b) << std::endl;

	} catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
	}
}
