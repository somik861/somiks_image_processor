#include "resize.hpp"
#include "common_macro.hpp"
#include <map>

namespace {
enum class interpolation_type { nn };

template <typename T>
T get_point_at(const ssimp::img::ndImage<T>& img,
               const std::vector<double>& coords,
               interpolation_type interp) {
	if (interp == interpolation_type::nn) {
		std::vector<std::size_t> closest(coords.size());
		std::ranges::transform(coords, closest.begin(), [](auto x) {
			return std::size_t(std::size_t(x));
		});

		return img(closest);
	}

	std::unreachable();
}

template <typename T>
ssimp::img::ndImage<T> resize_to(const ssimp::img::ndImage<T>& img,
                                 const std::vector<std::size_t>& new_dims,
                                 interpolation_type interp) {
	ssimp::img::ndImage<T> new_img(new_dims);

	std::vector<std::size_t> current_coords(new_dims.size());
	auto increment_coords = [&]() {
		++current_coords[0];
		for (std::size_t i = 0; i < new_dims.size(); ++i) {
			if (current_coords[i] < new_dims[i])
				break;
			current_coords[i] = 0;
			if (i + 1 < current_coords.size())
				++current_coords[i + 1];
		}
	};

	std::vector<double> coords_mult(new_dims.size());
	std::ranges::transform(
	    img.dims(), new_dims, coords_mult.begin(),
	    [](auto old, auto new_) { return double(old) / double(new_); });

	std::vector<double> mapped_coords(new_dims.size());
	do {
		std::ranges::transform(
		    current_coords, coords_mult, mapped_coords.begin(),
		    [](std::size_t coord, double mult) { return coord * mult; });

		new_img(current_coords) = get_point_at(img, mapped_coords, interp);

		increment_coords();
	} while (std::ranges::any_of(current_coords, [](auto x) { return x; }));

	return new_img;
}

} // namespace

namespace ssimp::algorithms {
bool Resize::image_count_supported(std::size_t count) { return count == 1; }
bool Resize::image_dims_supported(std::span<const std::size_t> dims) {
	return true;
}
bool Resize::same_dims_required() { return true; }

template <typename T>
    requires mt::traits::is_any_of_tuple_v<T, Resize::supported_types>
/* static */ std::vector<img::LocalizedImage>
Resize::apply(const std::vector<img::ndImage<T>>& imgs,
              const option_types::options_t& options) {

	auto& img_ = imgs[0];

	double factor = std::get<double>(options.at("scale_factor"));
	std::vector<std::size_t> new_dims;
	std::ranges::transform(
	    img_.dims(), std::back_inserter(new_dims), [factor](auto x) {
		    return std::max<std::size_t>(1, std::llround(x * factor));
	    });

	interpolation_type interp =
	    std::map<std::string, interpolation_type>{
	        {"nearest neighbour", interpolation_type::nn}}
	        .at(std::get<std::string>(options.at("interpolation")));

	return {{resize_to(img_, new_dims, interp)}};
}

INSTANTIATE_TEMPLATE(Resize, img::GRAY8);
INSTANTIATE_TEMPLATE(Resize, img::GRAY16);
INSTANTIATE_TEMPLATE(Resize, img::GRAY32);
INSTANTIATE_TEMPLATE(Resize, img::GRAY64);
INSTANTIATE_TEMPLATE(Resize, img::FLOAT);
INSTANTIATE_TEMPLATE(Resize, img::DOUBLE);
INSTANTIATE_TEMPLATE(Resize, img::GRAY8A);
INSTANTIATE_TEMPLATE(Resize, img::RGB8);
INSTANTIATE_TEMPLATE(Resize, img::RGBA8);

} // namespace ssimp::algorithms
