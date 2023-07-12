#include "to_gray8.hpp"
#include "common_macro.hpp"
#include <algorithm>
#include <limits>

namespace {
template <typename T>
    requires std::is_same_v<T, ssimp::img::FLOAT> ||
             std::is_same_v<T, ssimp::img::DOUBLE>
std::vector<ssimp::img::LocalizedImage>
_apply(const ssimp::img::ndImage<T>& img_,
       const ssimp::option_types::options_t& options) {
	ssimp::img::ndImage<ssimp::img::GRAY8> out(img_.dims());

	std::transform(img_.begin(), img_.end(), out.begin(), [](auto elem) {
		return ssimp::img::GRAY8(std::round(elem * 255));
	});

	return {ssimp::img::LocalizedImage{out}};
}

template <typename T>
    requires std::is_same_v<T, ssimp::img::GRAY8>
std::vector<ssimp::img::LocalizedImage>
_apply(const ssimp::img::ndImage<T>& img_,
       const ssimp::option_types::options_t& options) {
	return {ssimp::img::LocalizedImage{img_}};
}

template <typename T>
    requires std::is_same_v<T, ssimp::img::GRAY16> ||
             std::is_same_v<T, ssimp::img::GRAY32> ||
             std::is_same_v<T, ssimp::img::GRAY64>
std::vector<ssimp::img::LocalizedImage>
_apply(const ssimp::img::ndImage<T>& img_,
       const ssimp::option_types::options_t& options) {

	ssimp::img::ndImage<ssimp::img::GRAY8> out(img_.dims());
	bool rescale = std::get<bool>(options.at("rescale"));

	std::transform(img_.begin(), img_.end(), out.begin(), [=](auto elem) {
		if (!rescale)
			return ssimp::img::GRAY8(elem);
		return ssimp::img::GRAY8(elem / (uint64_t(1) << (sizeof(T) * 8 - 8)));
	});

	return {ssimp::img::LocalizedImage{out}};
}

template <typename T>
    requires std::is_same_v<T, ssimp::img::GRAY8A>
std::vector<ssimp::img::LocalizedImage>
_apply(const ssimp::img::ndImage<T>& img_,
       const ssimp::option_types::options_t& options) {
	ssimp::img::ndImage<ssimp::img::GRAY8> out(img_.dims());

	ssimp::img::GRAY8 bg(std::get<int32_t>(options.at("gray_bg")));

	std::transform(img_.begin(), img_.end(), out.begin(), [=](auto elem) {
		return ssimp::img::GRAY8(std::round(elem[0] / (elem[1] / 255.0) +
		                                    bg / (1 - (elem[1] / 255.0))));
	});

	return {ssimp::img::LocalizedImage{out}};
}

template <typename T>
    requires std::is_same_v<T, ssimp::img::RGB8>
std::vector<ssimp::img::LocalizedImage>
_apply(const ssimp::img::ndImage<T>& img_,
       const ssimp::option_types::options_t& options) {
	ssimp::img::ndImage<ssimp::img::GRAY8> out(img_.dims());

	double red_mult = std::get<double>(options.at("red_mult"));
	double green_mult = std::get<double>(options.at("green_mult"));
	double blue_mult = std::get<double>(options.at("blue_mult"));

	std::transform(img_.begin(), img_.end(), out.begin(), [=](auto elem) {
		return ssimp::img::GRAY8(std::round(
		    elem[0] * red_mult + elem[1] * red_mult + elem[2] * red_mult));
	});

	return {ssimp::img::LocalizedImage{out}};
}

template <typename T>
    requires std::is_same_v<T, ssimp::img::RGBA8>
std::vector<ssimp::img::LocalizedImage>
_apply(const ssimp::img::ndImage<T>& img_,
       const ssimp::option_types::options_t& options) {
	ssimp::img::ndImage<ssimp::img::RGB8> out(img_.dims());

	ssimp::img::GRAY8 red_bg(std::get<int32_t>(options.at("red_bg")));
	ssimp::img::GRAY8 green_bg(std::get<int32_t>(options.at("green_bg")));
	ssimp::img::GRAY8 blue_bg(std::get<int32_t>(options.at("blue_bg")));

	std::transform(img_.begin(), img_.end(), out.begin(), [=](auto elem) {
		return ssimp::img::RGB8{
		    ssimp::img::GRAY8(std::round(elem[0] / (elem[3] / 255.0) +
		                                 red_bg / (1 - (elem[3] / 255.0)))),
		    ssimp::img::GRAY8(std::round(elem[1] / (elem[3] / 255.0) +
		                                 green_bg / (1 - (elem[3] / 255.0)))),
		    ssimp::img::GRAY8(std::round(elem[2] / (elem[3] / 255.0) +
		                                 blue_bg / (1 - (elem[3] / 255.0))))};
	});

	return _apply(out, options);
}

} // namespace

namespace ssimp::algorithms {
/*static*/ bool ToGray8::image_count_supported(std::size_t count) {
	return count == 1;
}
/* static */ bool
ToGray8::image_dims_supported(std::span<const std::size_t> dims) {
	return true;
}

/* static */ bool ToGray8::same_dims_required() { return true; }

template <typename T>
    requires mt::traits::is_any_of_tuple_v<T, ToGray8::supported_types>
/* static */ std::vector<img::LocalizedImage>
ToGray8::apply(const std::vector<img::ndImage<T>>& imgs,
               const option_types::options_t& options) {
	return _apply(imgs[0], options);
}

INSTANTIATE_TEMPLATE(ToGray8, img::GRAY8);
INSTANTIATE_TEMPLATE(ToGray8, img::GRAY8A);
INSTANTIATE_TEMPLATE(ToGray8, img::GRAY16);
INSTANTIATE_TEMPLATE(ToGray8, img::GRAY32);
INSTANTIATE_TEMPLATE(ToGray8, img::GRAY64);
INSTANTIATE_TEMPLATE(ToGray8, img::RGB8);
INSTANTIATE_TEMPLATE(ToGray8, img::RGBA8);
INSTANTIATE_TEMPLATE(ToGray8, img::FLOAT);
INSTANTIATE_TEMPLATE(ToGray8, img::DOUBLE);
} // namespace ssimp::algorithms
