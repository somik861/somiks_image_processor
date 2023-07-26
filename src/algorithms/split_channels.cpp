#include "split_channels.hpp"
#include "common_macro.hpp"
#include <algorithm>

namespace ssimp::algorithms {
/*static*/ bool SplitChannels::image_count_supported(std::size_t count) {
	return count == 1;
}
/* static */ bool
SplitChannels::image_dims_supported(std::span<const std::size_t> dims) {
	return true;
}

/* static */ bool SplitChannels::same_dims_required() { return false; }

template <typename T>
    requires mt::traits::is_any_of_tuple_v<T, SplitChannels::supported_types>
/* static */ std::vector<img::LocalizedImage>
SplitChannels::apply(const std::vector<img::ndImage<T>>& imgs,
                     const option_types::options_t& options) {
	std::vector<img::LocalizedImage> out;
	const auto& img_ = imgs[0];

	if constexpr (mt::traits::is_any_of_v<T, img::COMPLEX_F, img::COMPLEX_D>) {
		using prec_t = T::value_type;

		img::ndImage<prec_t> r(img_.dims());
		img::ndImage<prec_t> i(img_.dims());

		std::ranges::transform(img_, r.begin(),
		                       [=](auto elem) { return elem.real(); });

		std::ranges::transform(img_, i.begin(),
		                       [=](auto elem) { return elem.imag(); });

		out.push_back({r, "real"});
		out.push_back({i, "imaginary"});

	} else {
		for (std::size_t i = 0; i < mt::traits::array_size_v<T>; ++i) {
			img::ndImage<img::GRAY_8> ch(img_.dims());

			std::ranges::transform(img_, ch.begin(),
			                       [=](auto elem) { return elem[i]; });

			if constexpr (std::is_same_v<T, img::GRAYA_8>)
				out.push_back({ch, std::array{"gray", "alpha"}[i]});
			else
				out.push_back(
				    {ch, std::array{"red", "green", "blue", "alpha"}[i]});
		}
	}

	return out;
}

INSTANTIATE_TEMPLATE(SplitChannels, img::GRAYA_8);
INSTANTIATE_TEMPLATE(SplitChannels, img::RGB_8);
INSTANTIATE_TEMPLATE(SplitChannels, img::RGBA_8);
INSTANTIATE_TEMPLATE(SplitChannels, img::COMPLEX_F);
INSTANTIATE_TEMPLATE(SplitChannels, img::COMPLEX_D);
} // namespace ssimp::algorithms
