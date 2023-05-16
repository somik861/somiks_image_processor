#include "image.hpp"
#include "meta_types.hpp"
#include <iostream>

int main() {
	img::ndImage<img::GRAY8> image(1, 2, 4, 5, 6);
	img::ndImage<img::GRAY8> image2(1, 2, 4, 5, 6);

	for (auto dim : image.dims())
		std::cout << dim << ' ';
	std::cout << '\n';

	std::cout << std::boolalpha << (image.dims() == image2.dims()) << '\n';
}
