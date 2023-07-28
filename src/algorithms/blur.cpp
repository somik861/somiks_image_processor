#include "blur.hpp"
#include "common_macro.hpp"

namespace {
enum class boundary_condition { zero, mirror, nearest };

template <typename T>
T _get_blurred(const ssimp::img::ndImage<T>& img,
               const std::vector<std::size_t>& coords,
               std::size_t dim_idx,
               const std::vector<double>& right_kernel,
               boundary_condition bound) {

	std::size_t kernel_size = right_kernel.size();

	std::array<double, 4> accum{};

	auto coords_ = coords;
	for (int dx = -int(kernel_size) + 1; dx < int(kernel_size); ++dx) {
		int new_coord = int(coords[dim_idx]) + dx;
		int max = img.dims()[dim_idx];

		if (!(0 <= new_coord && new_coord < max)) {

			if (bound == boundary_condition::zero)
				continue;

			if (bound == boundary_condition::mirror) {
				if (new_coord < 0)
					new_coord = std::abs(new_coord);
				if (new_coord >= max)
					new_coord = 2 * max - new_coord - 1;

				// kernel does not fit
				if (new_coord < 0)
					return img(coords);
			}
			if (bound == boundary_condition::nearest)
				new_coord = std::clamp(new_coord, 0, max - 1);
		}
		coords_[dim_idx] = new_coord;
		T dx_val = img(coords_);
		double mult = right_kernel[std::abs(dx)];
		if constexpr (std::is_scalar_v<T>) {
			accum[0] += dx_val * mult;
		} else if constexpr (ssimp::mt::traits::is_complex_v<T>) {
			accum[0] += dx_val.real() * mult;
			accum[1] += dx_val.imag() * mult;
		} else {
			for (std::size_t i = 0; i < dx_val.size(); ++i)
				accum[i] += dx_val[i] * mult;
		}
	}

	if constexpr (std::is_scalar_v<T>)
		return T(accum[0]);
	else if constexpr (ssimp::mt::traits::is_complex_v<T>)
		return T(accum[0], accum[1]);
	else {
		T out{};
		for (std::size_t i = 0; i < out.size(); ++i)
			out[i] = typename T::value_type(accum[i]);

		return out;
	}
}

inline std::vector<double> _gauss_right_kernel(double sigma) {
	if (sigma < 10e-5)
		return {1.0};
	std::vector<double> out(std::ceil(3 * sigma) + 1);
	for (std::size_t i = 0; i < out.size(); ++i)
		out[i] = std::exp(-std::pow(double(i), 2) / (2 * std::pow(sigma, 2)));

	double sum = 2 * std::reduce(std::next(out.begin()), out.end()) + out[0];
	std::ranges::transform(out, out.begin(), [sum](auto x) { return x / sum; });
	return out;
}

inline std::vector<double> _box_right_kernel(double size) {
	std::size_t kernel_size = (std::size_t(size) / 2 * 2 + 1) / 2 + 1;
	return std::vector<double>(kernel_size, 1.0 / double(2 * kernel_size - 1));
}

template <typename T>
ssimp::img::ndImage<T> blur_dim(const ssimp::img::ndImage<T>& img,
                                std::size_t dim_idx,
                                const std::vector<double>& right_kernel,
                                boundary_condition bound) {
	ssimp::img::ndImage<T> new_img(img.dims());

	new_img.transform([&](auto _, const auto& current_coords) {
		return _get_blurred(img, current_coords, dim_idx, right_kernel, bound);
	});

	return new_img;
}

} // namespace

namespace ssimp::algorithms {
bool Blur::image_count_supported(std::size_t count) { return count == 1; }
bool Blur::image_dims_supported(std::span<const std::size_t> dims) {
	return dims.size() > 0 &&
	       std::ranges::all_of(dims, [](auto x) { return x > 0; });
}
bool Blur::same_dims_required() { return true; }

template <typename T>
    requires mt::traits::is_any_of_tuple_v<T, Blur::supported_types>
/* static */ std::vector<img::LocalizedImage>
Blur::apply(const std::vector<img::ndImage<T>>& imgs,
            const option_types::options_t& options) {
	auto img_ = imgs[0];

	double intensity = std::get<double>(options.at("intensity"));
	std::string filter = std::get<std::string>(options.at("filter"));

	std::vector<double> kernel =
	    (filter == "box" ? _box_right_kernel(intensity)
	                     : _gauss_right_kernel(intensity));

	boundary_condition bound =
	    std::unordered_map<std::string, boundary_condition>{
	        {"zero", boundary_condition::zero},
	        {"nearest", boundary_condition::nearest},
	        {"mirror", boundary_condition::mirror}}
	        .at(std::get<std::string>(options.at("bound_condition")));

	if (std::get<bool>(options.at("one_dim")))
		return {{blur_dim(img_, std::get<int32_t>(options.at("dim_idx")),
		                  kernel, bound)}};

	for (std::size_t dim = 0; dim < img_.dims().size(); ++dim)
		img_ = blur_dim(img_, dim, kernel, bound);

	return {{img_}};
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
