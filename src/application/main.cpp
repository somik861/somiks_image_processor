#include "extension_matcher.hpp"
#include "image.hpp"
#include "meta_types.hpp"
#include <iostream>

int main() {
	ExtensionMatcher ext_match;
	ext_match.register_extension("tiff", "tif");
	ext_match.register_extension("tiff", "tiff");
	ext_match.register_extension("jpeg", "jpg");

	for (auto m : ext_match.sorted_formats_by_priority("img.jpg"))
		std::cout << m << '\n';
}
