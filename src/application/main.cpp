#include "api.hpp"
#include <filesystem>
#include <format>
#include <iostream>

namespace fs = std::filesystem;

int main() {

	try {

		ssimp::API api;

		std::cout << api.get_properties("lenna.jpg");

		auto img = api.load_image("lenna.jpg")[0];
		img.location = "lenna2.tiff";

		api.save_image(img, ".", "jpeg", {});

	} catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
	}
}
