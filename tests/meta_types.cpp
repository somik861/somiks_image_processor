#include "../src/application/meta_types.hpp"
#include "common.hpp"
#include <tuple>

TEST_CASE("Meta traits") {
	SECTION("Type subset") {
		REQUIRE(mt::traits::is_subset_of_v<std::tuple<>, std::tuple<>>);
		REQUIRE(mt::traits::is_subset_of_v<std::tuple<>, std::tuple<int>>);
		REQUIRE(mt::traits::is_subset_of_v<std::tuple<int>, std::tuple<int>>);
		REQUIRE(!mt::traits::is_subset_of_v<std::tuple<int>, std::tuple<>>);
		REQUIRE(!mt::traits::is_subset_of_v<std::tuple<char>, std::tuple<int>>);
	}
}
