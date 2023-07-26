#include "change_type.hpp"
#include "common_conversions.hpp"
#include "common_macro.hpp"
#include <algorithm>
#include <limits>
#include <utility>

namespace ssimp::algorithms {
/*static*/ bool ChangeType::image_count_supported(std::size_t count) {
	return count == 1;
}
/* static */ bool
ChangeType::image_dims_supported(std::span<const std::size_t> dims) {
	return true;
}

/* static */ bool ChangeType::same_dims_required() { return true; }

template <typename T>
    requires mt::traits::is_any_of_tuple_v<T, ChangeType::supported_types>
/* static */ std::vector<img::LocalizedImage>
ChangeType::apply(const std::vector<img::ndImage<T>>& imgs,
                  const option_types::options_t& options) {
	const auto& img_ = imgs[0];

	std::string output_type = std::get<std::string>(options.at("output_type"));
	bool rescale = std::get<bool>(options.at("rescale"));
	img::GRAY_8 gray_bg(std::get<int32_t>(options.at("gray_bg")));
	img::RGB_8 rgb_bg{img::GRAY_8(std::get<int32_t>(options.at("red_bg"))),
	                  img::GRAY_8(std::get<int32_t>(options.at("green_bg"))),
	                  img::GRAY_8(std::get<int32_t>(options.at("blue_bg")))};
	std::array<double, 3> rgb_mult{std::get<double>(options.at("red_mult")),
	                               std::get<double>(options.at("green_mult")),
	                               std::get<double>(options.at("blue_mult"))};

	if (output_type == "same as input")
		return {{img_}};
	if (output_type == to_string(img::elem_type::GRAY_8))
		return {{conversions::all_to_all<img::GRAY_8>(img_, rescale, rgb_mult,
		                                              gray_bg, rgb_bg)}};
	if (output_type == to_string(img::elem_type::GRAY_16))
		return {{conversions::all_to_all<img::GRAY_16>(img_, rescale, rgb_mult,
		                                               gray_bg, rgb_bg)}};
	if (output_type == to_string(img::elem_type::GRAY_32))
		return {{conversions::all_to_all<img::GRAY_32>(img_, rescale, rgb_mult,
		                                               gray_bg, rgb_bg)}};
	if (output_type == to_string(img::elem_type::GRAY_64))
		return {{conversions::all_to_all<img::GRAY_64>(img_, rescale, rgb_mult,
		                                               gray_bg, rgb_bg)}};
	if (output_type == to_string(img::elem_type::FLOAT))
		return {{conversions::all_to_all<img::FLOAT>(img_, rescale, rgb_mult,
		                                             gray_bg, rgb_bg)}};
	if (output_type == to_string(img::elem_type::DOUBLE))
		return {{conversions::all_to_all<img::DOUBLE>(img_, rescale, rgb_mult,
		                                              gray_bg, rgb_bg)}};
	if (output_type == to_string(img::elem_type::GRAYA_8))
		return {{conversions::all_to_all<img::GRAYA_8>(img_, rescale, rgb_mult,
		                                               gray_bg, rgb_bg)}};
	if (output_type == to_string(img::elem_type::RGB_8))
		return {{conversions::all_to_all<img::RGB_8>(img_, rescale, rgb_mult,
		                                             gray_bg, rgb_bg)}};
	if (output_type == to_string(img::elem_type::RGBA_8))
		return {{conversions::all_to_all<img::RGBA_8>(img_, rescale, rgb_mult,
		                                              gray_bg, rgb_bg)}};
	if (output_type == to_string(img::elem_type::COMPLEX_F))
		return {{conversions::all_to_all<img::COMPLEX_F>(
		    img_, rescale, rgb_mult, gray_bg, rgb_bg)}};

	if (output_type == to_string(img::elem_type::COMPLEX_D))
		return {{conversions::all_to_all<img::COMPLEX_D>(
		    img_, rescale, rgb_mult, gray_bg, rgb_bg)}};

	std::unreachable();
}

INSTANTIATE_TEMPLATE(ChangeType, img::GRAY_8);
INSTANTIATE_TEMPLATE(ChangeType, img::GRAYA_8);
INSTANTIATE_TEMPLATE(ChangeType, img::GRAY_16);
INSTANTIATE_TEMPLATE(ChangeType, img::GRAY_32);
INSTANTIATE_TEMPLATE(ChangeType, img::GRAY_64);
INSTANTIATE_TEMPLATE(ChangeType, img::RGB_8);
INSTANTIATE_TEMPLATE(ChangeType, img::RGBA_8);
INSTANTIATE_TEMPLATE(ChangeType, img::FLOAT);
INSTANTIATE_TEMPLATE(ChangeType, img::DOUBLE);
INSTANTIATE_TEMPLATE(ChangeType, img::COMPLEX_F);
INSTANTIATE_TEMPLATE(ChangeType, img::COMPLEX_D);
} // namespace ssimp::algorithms
