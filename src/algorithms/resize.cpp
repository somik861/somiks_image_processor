#include "resize.hpp"
#include "common_macro.hpp"
#include <boost/lexical_cast.hpp>
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

// TODO
template <typename T>
ssimp::img::ndImage<T>
blur_dims_clever(const ssimp::img::ndImage<T>& img,
                 const std::vector<std::size_t>& new_dims) {
	throw std::runtime_error("NOT IMPLEMENTED");
	return img;
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
void blur_dim(ssimp::img::ndImage<T>& img, std::size_t dim_idx, double sigma) {
	std::vector<double> kernel = gauss_right_kernel(sigma);

	std::vector<std::size_t> current_coords(img.dims().size());
	auto increment_coords = [&]() {
		++current_coords[0];
		for (std::size_t i = 0; i < current_coords.size(); ++i) {
			if (current_coords[i] < img.dims()[i])
				break;
			current_coords[i] = 0;
			std::size_t next = i + 1;
			if (next == dim_idx) {
				++next;
				++i;
			}
			if (next < current_coords.size())
				++current_coords[next];
		}
	};

	do {
		// TODO

		increment_coords();
		current_coords[dim_idx] = 0;
	} while (std::ranges::any_of(current_coords, [](auto x) { return x; }));
}

template <typename T>
ssimp::img::ndImage<T>
blur_dims_fast(const ssimp::img::ndImage<T>& img,
               const std::vector<std::size_t>& new_dims) {
	ssimp::img::ndImage<T> new_img = img.copy();
	for (std::size_t i = 0; i < new_dims.size(); ++i) {
		double down_factor = double(new_dims[i]) / double(img.dims()[i]);
		if (down_factor > 1.0)
			blur_dim(new_img, i, (down_factor - 1.0) / 2.0);
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
