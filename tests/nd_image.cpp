#include "../src/application/nd_image.hpp"
#include "common.hpp"
#include <tuple>

using scalar_type_list = std::tuple<img::GRAY_8,
                                    img::GRAY_16,
                                    img::GRAY_32,
                                    img::GRAY_64,
                                    img::FLOAT,
                                    img::DOUBLE>;

TEMPLATE_LIST_TEST_CASE("ndImage", "ndImage[template]", scalar_type_list) {
	using T = TestType;

	img::ndImage<T> image(2, 3, 4);
	img::ndImage<T> image2(std::array<std::size_t, 2>{4, 4});

	img::ndImageBase image_base = image;
	img::ndImageBase image2_base = image2;

	SECTION("Basic attributes") {
		REQUIRE(image.dims() == image_base.dims());
		REQUIRE(image2.dims() == image2_base.dims());
		REQUIRE(image.type() == img::type_to_enum<T>);
		REQUIRE(image2.type() == img::type_to_enum<T>);
		REQUIRE(image_base.type() == img::type_to_enum<T>);
		REQUIRE(image2_base.type() == img::type_to_enum<T>);
	}

	SECTION("Element access") {
		image(0, 0, 0) = T(1);
		REQUIRE(image_base.as_typed<T>()(0, 0, 0) == T(1));

		image(1, 2, 2) = T(3);
		REQUIRE(image_base.as_typed<T>()(1, 2, 2) == T(3));
	}

	SECTION("Shallow copy") {
		img::ndImage<T> cpy = image;
		img::ndImageBase cpy_base = image_base;

		image(1, 2, 3) = T(4);
		REQUIRE(cpy(1, 2, 3) == T(4));
		REQUIRE(cpy_base.as_typed<T>()(1, 2, 3) == T(4));
	}

	SECTION("Deep copy") {
		img::ndImage<T> cpy = image.copy();
		img::ndImageBase cpy_base = image_base.copy();

		image(1, 2, 3) = T(4);
		REQUIRE(cpy(1, 2, 3) == T(0));
		REQUIRE(cpy_base.as_typed<T>()(1, 2, 3) == T(0));
	}

	SECTION("Iterators") {
		uint8_t start = 1;
		auto generator = [&start]() -> T { return T(start++); };
		std::ranges::generate(image, generator);

		REQUIRE(image(0, 0, 0) == T(1));
		REQUIRE(image(1, 0, 0) == T(2));
		REQUIRE(image(0, 2, 3) == T(23));
		REQUIRE(image(1, 2, 3) == T(24));
	}

	SECTION("Span") {
		uint8_t start = 1;
		auto generator = [&start]() { return T(start++); };
		std::ranges::generate(image, generator);

		REQUIRE(image(0, 0, 0) == T(1));
		REQUIRE(image(1, 0, 0) == T(2));
		REQUIRE(image(0, 2, 3) == T(23));
		REQUIRE(image(1, 2, 3) == T(24));
	}
}
