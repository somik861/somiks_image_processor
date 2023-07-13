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

	for (std::size_t i = 0; i < mt::traits::array_size_v<T>; ++i) {
		img::ndImage<img::GRAY8> ch(img_.dims());

		std::transform(img_.begin(), img_.end(), ch.begin(),
		               [=](auto elem) { return elem[i]; });

		if constexpr (std::is_same_v<T, img::GRAY8A>)
			out.push_back({ch, std::array{"gray", "alpha"}[i]});
		else
			out.push_back({ch, std::array{"red", "green", "blue", "alpha"}[i]});
	}

	return out;
}

INSTANTIATE_TEMPLATE(SplitChannels, img::GRAY8A);
INSTANTIATE_TEMPLATE(SplitChannels, img::RGB8);
INSTANTIATE_TEMPLATE(SplitChannels, img::RGBA8);
} // namespace ssimp::algorithms
