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

template <typename T>
    requires mt::traits::is_any_of_tuple_v<T, SplitChannels::supported_types>
/* static */ std::vector<img::LocalizedImage>
SplitChannels::apply(const std::vector<img::ndImage<T>>& imgs,
                     const option_types::options_t& options) {
	std::vector<img::LocalizedImage> out;
	const auto& img_ = imgs;

	img::ndImage<img::GRAY8> r(imgs.dims()), g(imgs.dims()), b(imgs.dims());

	std::transform(img_.begin(), img_.end(), r.begin(),
	               [](auto elem) { return elem[0]; });
	std::transform(img_.begin(), img_.end(), g.begin(),
	               [](auto elem) { return elem[1]; });
	std::transform(img_.begin(), img_.end(), b.begin(),
	               [](auto elem) { return elem[2]; });

	out.push_back({r, "r"});
	out.push_back({g, "g"});
	out.push_back({b, "b"});

	if constexpr (std::is_same_v<T, img::RGBA8>) {
		img::ndImage<img::GRAY8> a(imgs.dims());

		std::transform(img_.begin(), img_.end(), a.begin(),
		               [](auto elem) { return elem[3]; });
		out.push_back({a, "a"});
	}

	return out;
}

INSTANTIATE_TEMPLATE(SplitChannels, img::RGB8);
INSTANTIATE_TEMPLATE(SplitChannels, img::RGBA8);
} // namespace ssimp::algorithms
