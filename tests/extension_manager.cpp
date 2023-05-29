#include "../src/application/managers/extension_manager.hpp"
#include "common.hpp"
#include <initializer_list>

TEST_CASE("ExtensionManager") {
	ExtensionManager ext_match;

	SECTION("Raw matching") {
		ext_match.register_extension("tiff", "tif");
		ext_match.register_extension("jpeg", "jpg");

		REQUIRE(ext_match.find_possible_formats("img.jpg") ==
		        std::vector{"jpeg"s});

		REQUIRE(ext_match.sorted_formats_by_priority("img.jpg") ==
		        std::vector{"jpeg"s, "tiff"s});
	}

	SECTION("Regex matching") {
		ext_match.register_extension("tiff", "tiff?", true);
		ext_match.register_extension("jpeg", "jpg");
		ext_match.register_extension("png", "png.",
		                             true); // this is not real :D

		for (std::string img : {"img.tif", "img.tiff"})
			REQUIRE(ext_match.find_possible_formats(img) ==
			        std::vector{"tiff"s});

		for (std::string img : {"img.tif", "img.tiff"})
			REQUIRE(ext_match.sorted_formats_by_priority(img) ==
			        std::vector{"tiff"s, "jpeg"s, "png"s});
	}

	SECTION("Format manimpulation operators") {
		auto test_contains =
		    [&ext_match](std::initializer_list<std::string> il) {
			    std::unordered_set<std::string> expected(il.begin(), il.end());
			    REQUIRE(ext_match.registered_formats() == expected);
		    };

		test_contains({});

		ext_match.register_extension("jpeg", "jpg");
		test_contains({"jpeg"});

		ext_match.register_extension("tiff", "tiff?", true);
		test_contains({"jpeg", "tiff"});

		ext_match.remove_extension("jpeg", "jpg");
		test_contains({"tiff"});

		ext_match.register_extension("png", "png");
		test_contains({"tiff", "png"});

		ext_match.remove_extension("tiff", "tiff?", true);
		test_contains({"png"});

		ext_match.register_extension("jpeg", "jpg");
		ext_match.register_extension("jpeg", "jpg");
		ext_match.register_extension("jpeg", "jpg");
		ext_match.remove_format("png");
		test_contains({"jpeg"});
		REQUIRE(ext_match.format_raw_suffixes("jpeg") == std::vector{"jpg"s});
	}
}
