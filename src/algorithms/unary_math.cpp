#include "unary_math.hpp"
#include "common_macro.hpp"
#include <boost/multiprecision/cpp_int.hpp>
#include <cmath>
#include <cstdlib>

namespace ssimp::algorithms {
bool UnaryMath::image_count_supported(std::size_t count) { return count == 1; }
bool UnaryMath::image_dims_supported(std::span<const std::size_t> dims) {
	return dims.size() > 0 &&
	       std::ranges::all_of(dims, [](auto x) { return x > 0; });
}
bool UnaryMath::same_dims_required() { return true; }

template <typename T>
    requires mt::traits::is_any_of_tuple_v<T, UnaryMath::supported_types>
/* static */ std::vector<img::LocalizedImage>
UnaryMath::apply(const std::vector<img::ndImage<T>>& imgs,
                 const option_types::options_t& options) {
	using boost::multiprecision::uint128_t;

	std::string function = std::get<std::string>(options.at("function"));
	auto& img_ = imgs[0];

	std::vector<img::LocalizedImage> out;

	if (function == "identity")
		out.push_back({img_});

	if (function == "linear_stretch") {
		if constexpr (mt::traits::is_complex_v<T>)
			out.push_back({img_});
		else {
			img::ndImage<T> out_img(img_.dims());
			T max = std::numeric_limits<T>::max();
			T min = std::numeric_limits<T>::min();
			if constexpr (std::is_floating_point_v<T>) {
				min = 0.0;
				max = 1.0;
			}
			T range = max - min;

			auto [input_min, input_max] = std::ranges::minmax(img_);
			T input_range = input_max - input_min;

			std::ranges::transform(img_, out_img.begin(), [=](T x) {
				if constexpr (std::is_floating_point_v<T>)
					return (x - input_min) / input_range * range + min;
				else {
					uint128_t x_ = x;
					return T((x_ - input_min) * range / input_range + min);
				}
			});
			out.push_back({out_img});
		}
	}

	if (function == "abs") {
		if constexpr (std::is_unsigned_v<T>) {
			out.push_back({img_});
		} else {
			img::ndImage<decltype(std::abs(T{}))> out_img(img_.dims());
			std::ranges::transform(img_, out_img.begin(),
			                       [](auto x) { return std::abs(x); });
			out.push_back({out_img});
		}
	}

	if (function == "negative") {
		if constexpr (mt::traits::is_complex_v<T>) {
			out.push_back({img_});
		} else {
			img::ndImage<T> out_img(img_.dims());
			T max = std::numeric_limits<T>::max();
			if constexpr (std::is_floating_point_v<T>)
				max = 1.0;

			std::ranges::transform(img_, out_img.begin(),
			                       [=](auto x) { return max - x; });
			out.push_back({out_img});
		}
	}

	return out;
}

INSTANTIATE_TEMPLATE(UnaryMath, img::GRAY_8);
INSTANTIATE_TEMPLATE(UnaryMath, img::GRAY_16);
INSTANTIATE_TEMPLATE(UnaryMath, img::GRAY_32);
INSTANTIATE_TEMPLATE(UnaryMath, img::GRAY_64);
INSTANTIATE_TEMPLATE(UnaryMath, img::FLOAT);
INSTANTIATE_TEMPLATE(UnaryMath, img::DOUBLE);
INSTANTIATE_TEMPLATE(UnaryMath, img::COMPLEX_F);
INSTANTIATE_TEMPLATE(UnaryMath, img::COMPLEX_D);

} // namespace ssimp::algorithms
