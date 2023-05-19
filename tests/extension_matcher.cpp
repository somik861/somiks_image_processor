#include "../src/application/extension_matcher.hpp"
#include "common.hpp"

TEST_CASE("ExtensionMatcher") {
	ExtensionMatcher ext_match;

	SECTION("Raw matching") {
		ext_match.register_extension("tiff", "tif");
		ext_match.register_extension("jpeg", "jpg");

		std::vector<std::string> formats =
		    ext_match.sorted_formats_by_priority("img.jpg");

		REQUIRE(formats.size() == 2);
		REQUIRE(formats[0] == "jpg");
		REQUIRE(formats[1] == "tiff");
	}
}
