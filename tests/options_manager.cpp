#include "../src/application/managers/options_manager.hpp"
#include "../src/application/managers/config_manager.hpp"
#include "common.hpp"

TEST_CASE("OptionManager") {
	SECTION("Load sample format") {
		ConfigManager cfg;
		auto loaded = cfg.load_format("testing_sample");

		OptionsManager options;
		options.load_from_json("test",
		                       loaded.get_object().at("options").get_array());

		// Basic option testing
		REQUIRE(options.is_valid("test", {}));
		REQUIRE(options.is_valid("test", {{"compression", "None"}}));
		REQUIRE(options.is_valid("test", {{"compression", "LZW"}}));
		REQUIRE(!options.is_valid("test", {{"compression", "ZLIB"}}));

		// Invalid option or values
		REQUIRE(!options.is_valid("test", {{"xyz", 1}}));
		REQUIRE(!options.is_valid("test", {{"quality_perc", 50.0}}));
		REQUIRE(!options.is_valid(
		    "test", {{"quality_perc", 50}, {"quality_loss", true}}));
		REQUIRE(!options.is_valid(
		    "test", {{"quality_perc", 150.0}, {"quality_loss", true}}));
		REQUIRE(options.is_valid(
		    "test", {{"quality_perc", 50.0}, {"quality_loss", true}}));

		// Options finalizer
		auto final1 = options.finalize_options("test", {});
		REQUIRE(std::get<std::string>(final1.at("compression")) == "None");
		REQUIRE(!final1.contains("quality_perc"));
		REQUIRE(!std::get<bool>(final1.at("quality_loss")));

		auto final2 =
		    options.finalize_options("test", {{"compression", "LZW"}});

		REQUIRE(std::get<std::string>(final2.at("compression")) == "LZW");
		REQUIRE(!final2.contains("quality_perc"));
		REQUIRE(!std::get<bool>(final2.at("quality_loss")));

		auto final3 =
		    options.finalize_options("test", {{"quality_loss", true}});

		REQUIRE(std::get<std::string>(final3.at("compression")) == "None");
		REQUIRE(std::get<bool>(final3.at("quality_loss")));
		REQUIRE(std::get<double>(final3.at("quality_perc")) == 100.0);
	}
}
