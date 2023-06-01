#include "api.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int main() {
	ssimp::API api;
	auto imgs = api.load_image("testing_image.img");

	std::cout << "Image count: " << imgs.size() << '\n';
	auto img = imgs[0];
	std::cout << "Images[0]:\n" << img;
}
