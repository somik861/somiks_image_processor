#include "api.hpp"
#include <filesystem>
#include <format>
#include <iostream>
#include <string>

namespace fs = std::filesystem;
using namespace std::literals;

int main() {

	try {
		ssimp::API api;
		auto img = api.load_one("C:\\Users\\janju\\Desktop\\lena_gray.png");
		img = api.apply({img}, "change_type", {{"output_type", "DOUBLE"}})[0];
		img = api.apply({img}, "fft")[0];

		auto res = img.image.as_typed<ssimp::img::COMPLEX_D>();
		std::cout << "Result: ";
		for (std::size_t i = 0; i < 5; ++i)
			std::cout << res(i) << " ";
		std::cout << '\n';
	} catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
	}
}
