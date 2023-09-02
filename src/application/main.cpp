#include "api.hpp"
#include <fftw3.h>
#include <filesystem>
#include <format>
#include <iostream>
#include <string>

namespace fs = std::filesystem;
using namespace std::literals;

namespace {
void print_first_five(auto name, auto img_) {
	std::cout << name << ": ";
	if (img_.type() == ssimp::img::elem_type::DOUBLE)
		for (std::size_t i = 0; i < 5; ++i)
			std::cout << img_.as_typed<ssimp::img::DOUBLE>()(i) << " ";
	if (img_.type() == ssimp::img::elem_type::COMPLEX_D)
		for (std::size_t i = 0; i < 5; ++i)
			std::cout << img_.as_typed<ssimp::img::COMPLEX_D>()(i) << " ";
	std::cout << '\n';
}
} // namespace

int main() {

	try {
		ssimp::API api;
		auto img = api.load_one("C:\\Users\\janju\\Desktop\\lena_gray.png");
		img = api.apply({img}, "change_type",
		                {{"output_type", "COMPLEX_D"}, {"rescale", false}})[0];

		print_first_five("BEFORE FFT", img.image);

		img = api.apply({img}, "fft")[0];

		print_first_five("AFTER FFT", img.image);

		img = api.apply({img}, "fft", {{"direction", "backward"}})[0];

		print_first_five("AFTER IFFT", img.image);

	} catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
	}
}
