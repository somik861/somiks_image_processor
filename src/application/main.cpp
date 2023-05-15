#include "image.hpp"
#include "meta_types.hpp"
#include <iostream>

int main() {
	using img_types = img::type_list;

	std::cout << mt::traits::tuple_type_idx_v<uint8_t, img_types> << '\n';
}
