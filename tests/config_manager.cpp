#include "../src/application/managers/config_manager.hpp"
#include "common.hpp"

TEST_CASE("ConfigManager") {
	SECTION("Load sample format") {
		ConfigManager cfg;
		auto loaded = cfg.load_format("testing_sample");

		REQUIRE(loaded.is_object());
	}
}
