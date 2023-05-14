#include "image.hpp"
#include "meta_types.hpp"
#include <iostream>

int main() {
	using img_types = img::type_list;

	std::cout << std::boolalpha
	          << mt::traits::tuple_type_idx_v<uint16_t, img_types> << '\n';
}
