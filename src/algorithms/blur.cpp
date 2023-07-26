#include "blur.hpp"
#include "common_macro.hpp"

namespace ssimp::algorithms {
bool Blur::image_count_supported(std::size_t count) { return false; }
bool Blur::image_dims_supported(std::span<const std::size_t> dims) {
	return false;
}
bool Blur::same_dims_required() { return false; }

template <typename T>
    requires mt::traits::is_any_of_tuple_v<T, Blur::supported_types>
/* static */ std::vector<img::LocalizedImage>
Blur::apply(const std::vector<img::ndImage<T>>& imgs,
            const option_types::options_t& options) {
	// TODO
	return {};
}

INSTANTIATE_TEMPLATE(Blur, img::GRAY_8);
INSTANTIATE_TEMPLATE(Blur, img::GRAY_16);
INSTANTIATE_TEMPLATE(Blur, img::GRAY_32);
INSTANTIATE_TEMPLATE(Blur, img::GRAY_64);
INSTANTIATE_TEMPLATE(Blur, img::GRAYA_8);
INSTANTIATE_TEMPLATE(Blur, img::RGB_8);
INSTANTIATE_TEMPLATE(Blur, img::RGBA_8);
INSTANTIATE_TEMPLATE(Blur, img::FLOAT);
INSTANTIATE_TEMPLATE(Blur, img::DOUBLE);
INSTANTIATE_TEMPLATE(Blur, img::COMPLEX_F);
INSTANTIATE_TEMPLATE(Blur, img::COMPLEX_D);

} // namespace ssimp::algorithms
