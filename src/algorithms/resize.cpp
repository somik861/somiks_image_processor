#include "resize.hpp"
#include "common_conversions.hpp"
#include "common_macro.hpp"
#include "fft.hpp"
#include <boost/lexical_cast.hpp>
#include <iostream>
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

std::vector<std::string> split(const std::string& str, char delim) {
	std::vector<std::string> out;
	std::string* buffer = &out.emplace_back();
	for (char ch : str) {
		if (ch == delim)
			buffer = &out.emplace_back();
		else
			buffer->push_back(ch);
	}
	return out;
}

std::string strip_ws(const std::string& str) {
	auto it = str.begin();
	auto end_it = str.end();
	while (it != end_it && std::isspace(*it))
		++it;

	std::string out(it, end_it);
	while (!out.empty() && std::isspace(out.back()))
		out.pop_back();

	return out;
}

std::vector<std::size_t> string_to_dims(const std::string& str) {
	std::vector<std::size_t> out;
	for (const auto& val : split(str, ','))
		out.push_back(boost::lexical_cast<std::size_t>(strip_ws(val)));

	return out;
}

std::vector<std::size_t> scale_dims(const std::vector<std::size_t>& old,
                                    double factor) {
	std::vector<std::size_t> new_(old.size());
	std::ranges::transform(old, new_.begin(), [factor](auto x) {
		return std::max<std::size_t>(1, std::llround(x * factor));
	});
	return new_;
}

template <typename T>
ssimp::img::ndImage<T>
blur_dims_clever_help(const ssimp::img::ndImage<T>& img,
                      const std::vector<std::size_t>& new_dims) {
	auto img_cdouble =
	    ssimp::algorithms::conversions::all_to_all<ssimp::img::COMPLEX_D>(
	        img, false, {}, {}, {});

	auto fft_img = ssimp::algorithms::FFT::apply(std::vector{img_cdouble},
	                                             {{"direction", "forward"},
	                                              {"shift", false},
	                                              {"normalize", true}})[0]
	                   .image.template as_typed<ssimp::img::COMPLEX_D>();

	std::vector<std::size_t> dim_even(img.dims().size());
	std::ranges::transform(img.dims(), dim_even.begin(),
	                       [](auto x) { return 1 - (x % 2); });

	std::vector<std::size_t> dim_mid(img.dims().size());
	std::ranges::transform(img.dims(), dim_even.begin(),
	                       [](auto x) { return x / 2 + x % 2; });

	std::vector<std::size_t> dim_offset(img.dims().size());
	for (std::size_t i = 0; i < dim_offset.size(); ++i)
		dim_offset[i] = img.dims()[i] >= new_dims[i]
		                    ? 0
		                    : (img.dims()[i] - new_dims[i]) / 2;

	for (std::size_t dim = 0; dim < new_dims.size(); ++dim)
		fft_img.transform_rows(dim, [&](auto proxy) {
			if (dim_offset[dim] == 0)
				return;
			for (std::size_t i = dim_mid[dim] - dim_offset[dim];
			     i < dim_mid[dim] + dim_offset[dim] + dim_even[dim]; ++i)
				proxy[i] = 0;
		});

	auto ifft_img = ssimp::algorithms::FFT::apply(std::vector{img_cdouble},
	                                              {{"direction", "backward"},
	                                               {"shift", false},
	                                               {"normalize", true}})[0]
	                    .image.template as_typed<ssimp::img::DOUBLE>();

	return ssimp::algorithms::conversions::all_to_all<T>(ifft_img, false, {},
	                                                     {}, {});
}

template <typename T>
ssimp::img::ndImage<T>
blur_dims_clever(const ssimp::img::ndImage<T>& img,
                 const std::vector<std::size_t>& new_dims) {
	if constexpr (std::is_scalar_v<T>)
		return blur_dims_clever_help(img, new_dims);
	else {
		constexpr std::size_t channel_count =
		    ssimp::mt::traits::array_size_v<T>;
		std::vector<ssimp::img::ndImage<ssimp::img::GRAY_8>> channels;
		for (std::size_t ch = 0; ch < channel_count; ++ch) {
			channels.emplace_back(img.dims());
			std::ranges::transform(img, channels.back().begin(),
			                       [=](auto elem) { return elem[ch]; });
		}

		std::ranges::transform(channels, channels.begin(), [&](auto img_) {
			return blur_dims_clever_help(img_, new_dims);
		});

		ssimp::img::ndImage<T> out(img.dims());
		for (std::size_t i = 0; i < out.span().size(); ++i)
			for (std::size_t ch = 0; ch < channel_count; ++ch)
				out(i)[ch] = channels[ch](i);

		return out;
	}
}

std::vector<double> gauss_right_kernel(double sigma) {
	std::vector<double> out(std::ceil(3 * sigma) + 1);
	for (std::size_t i = 0; i < out.size(); ++i)
		out[i] = 1.0 / sigma *
		         std::exp(-std::pow(double(i), 2) / (2 * std::pow(sigma, 2)));

	double sum = 2 * std::reduce(std::next(out.begin()), out.end()) + out[0];
	std::ranges::transform(out, out.begin(), [sum](auto x) { return x / sum; });
	return out;
}

template <typename T>
T _get_blurred(const ssimp::img::ndImage<T>& img,
               const std::vector<std::size_t>& coords,
               std::size_t dim_idx,
               const std::vector<double>& gauss_right) {
	std::size_t kernel_size = gauss_right.size();

	std::array<double, 4> accum{};

	auto coords_ = coords;
	for (int dx = -int(kernel_size) + 1; dx < int(kernel_size); ++dx) {
		int new_coord = std::abs(int(coords[dim_idx]) + dx);
		if (new_coord >= coords[dim_idx])
			new_coord = 2 * int(coords[dim_idx]) - new_coord - 1;

		// kernel does not fit
		if (new_coord < 0) {
			return img(coords);
			continue;
		}

		coords_[dim_idx] = new_coord;
		T dx_val = img(coords_);
		if constexpr (std::is_scalar_v<T>) {
			accum[0] += dx_val * gauss_right[std::abs(dx)];
		} else {
			for (std::size_t i = 0; i < dx_val.size(); ++i)
				accum[i] += dx_val[i] * gauss_right[std::abs(dx)];
		}
	}

	if constexpr (std::is_scalar_v<T>)
		return T(accum[0]);
	else {
		T out{};
		for (std::size_t i = 0; i < out.size(); ++i)
			out[i] = typename T::value_type(accum[i]);

		return out;
	}
}

template <typename T>
ssimp::img::ndImage<T>
blur_dim(const ssimp::img::ndImage<T>& img, std::size_t dim_idx, double sigma) {
	std::vector<double> kernel = gauss_right_kernel(sigma);
	ssimp::img::ndImage<T> new_img(img.dims());

	std::vector<std::size_t> current_coords(img.dims().size());
	auto increment_coords = [&]() {
		++current_coords[0];
		for (std::size_t i = 0; i < current_coords.size(); ++i) {
			if (current_coords[i] < img.dims()[i])
				break;
			current_coords[i] = 0;
			std::size_t next = i + 1;
			if (next < current_coords.size())
				++current_coords[next];
		}
	};

	do {
		new_img(current_coords) =
		    _get_blurred(img, current_coords, dim_idx, kernel);
		increment_coords();
	} while (std::ranges::any_of(current_coords, [](auto x) { return x; }));

	return new_img;
}

template <typename T>
ssimp::img::ndImage<T>
blur_dims_fast(const ssimp::img::ndImage<T>& img,
               const std::vector<std::size_t>& new_dims) {
	ssimp::img::ndImage<T> new_img = img.copy();
	for (std::size_t i = 0; i < new_dims.size(); ++i) {
		double down_factor = double(img.dims()[i]) / double(new_dims[i]);
		if (down_factor > 1.0)
			new_img = blur_dim(new_img, i, (down_factor - 1.0) / 2.0);
	}
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

	auto img_ = imgs[0];

	// ====== COMPUTE OUTPUT RES
	double factor = std::get<double>(options.at("scale_factor"));
	std::vector<std::size_t> new_dims = scale_dims(img_.dims(), factor);

	std::string exact_res = std::get<std::string>(options.at("exact_res"));
	if (exact_res != "auto") {
		new_dims = string_to_dims(exact_res);
		if (new_dims.size() != img_.dims().size())
			throw exceptions::Unsupported(
			    "Given resolution does not have the same number of dimensions");

		std::size_t zero_c = std::ranges::count(new_dims, 0);
		if (zero_c > 0) {
			if (zero_c != new_dims.size() - 1)
				throw exceptions::Unsupported("To preserve aspect ratio, leave "
				                              "only one non-zero element");

			std::size_t non_zero_idx = std::distance(
			    new_dims.begin(),
			    std::ranges::find_if(new_dims, [](auto x) { return x != 0; }));

			new_dims =
			    scale_dims(img_.dims(), double(new_dims[non_zero_idx]) /
			                                double(img_.dims()[non_zero_idx]));
		}
	}

	// ====== APPLY ANTIALISING
	std::string anti_alias = std::get<std::string>(options.at("anti_aliasing"));
	if (anti_alias == "fast")
		img_ = blur_dims_fast(img_, new_dims);
	if (anti_alias == "clever")
		img_ = blur_dims_clever(img_, new_dims);

	interpolation_type interp =
	    std::map<std::string, interpolation_type>{
	        {"nearest neighbour", interpolation_type::nn}}
	        .at(std::get<std::string>(options.at("interpolation")));

	return {{resize_to(img_, new_dims, interp)}};
}

INSTANTIATE_TEMPLATE(Resize, img::GRAY_8);
INSTANTIATE_TEMPLATE(Resize, img::GRAY_16);
INSTANTIATE_TEMPLATE(Resize, img::GRAY_32);
INSTANTIATE_TEMPLATE(Resize, img::GRAY_64);
INSTANTIATE_TEMPLATE(Resize, img::FLOAT);
INSTANTIATE_TEMPLATE(Resize, img::DOUBLE);
INSTANTIATE_TEMPLATE(Resize, img::GRAYA_8);
INSTANTIATE_TEMPLATE(Resize, img::RGB_8);
INSTANTIATE_TEMPLATE(Resize, img::RGBA_8);

} // namespace ssimp::algorithms
