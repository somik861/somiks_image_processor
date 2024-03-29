#include "fft.hpp"
#include "common_conversions.hpp"
#include "common_macro.hpp"
#include <fftw3.h>

namespace {

template <typename T>
struct precision {
	using type = T;
};

template <typename T>
struct precision<std::complex<T>> {
	using type = T;
};

template <typename T>
using precision_t = precision<T>::type;

inline bool is_real(const ssimp::img::ndImage<ssimp::img::COMPLEX_D>& img) {
	return std::ranges::all_of(
	    img, [](auto x) { return std::abs(x.imag()) < 1e-6; });
}

inline void shift_image(ssimp::img::ndImage<ssimp::img::COMPLEX_D>& img,
                        bool to_center) {
	for (std::size_t dim = 0; dim < img.dims().size(); ++dim)
		img.transform_rows(dim, [=](auto proxy) {
			std::size_t mid = proxy.size() / 2;
			if (to_center)
				std::rotate(proxy.begin(), std::next(proxy.begin(), mid),
				            proxy.end());
			else
				std::rotate(proxy.rbegin(), std::next(proxy.rbegin(), mid),
				            proxy.rend());
		});
}
} // namespace

namespace ssimp::algorithms {
bool FFT::image_count_supported(std::size_t count) { return count == 1; }
bool FFT::image_dims_supported(std::span<const std::size_t> dims) {
	return dims.size() > 0 &&
	       std::ranges::all_of(dims, [](auto x) { return x > 0; });
}
bool FFT::same_dims_required() { return true; }

template <typename T>
    requires mt::traits::is_any_of_tuple_v<T, FFT::supported_types>
/* static */ std::vector<img::LocalizedImage>
FFT::apply(const std::vector<img::ndImage<T>>& imgs,
           const option_types::options_t& options) {
	int direction = std::get<std::string>(options.at("direction")) == "forward"
	                    ? FFTW_FORWARD
	                    : FFTW_BACKWARD;

	bool shift = std::get<bool>(options.at("shift"));
	bool normalize = std::get<bool>(options.at("normalize"));

	auto complex_in =
	    conversions::all_to_all<img::COMPLEX_D>(imgs[0], false, {}, {}, {});

	if (shift && direction == FFTW_BACKWARD)
		shift_image(complex_in, false);

	img::ndImage<img::COMPLEX_D> complex_out(complex_in.dims());
	int rank = int(complex_in.dims().size());
	std::vector<int> n(rank);
	std::ranges::transform(complex_in.dims(), n.begin(),
	                       [](auto x) { return int(x); });

	fftw_plan plan = fftw_plan_dft(
	    rank, n.data(), reinterpret_cast<fftw_complex*>(complex_in.data()),
	    reinterpret_cast<fftw_complex*>(complex_out.data()), direction,
	    FFTW_ESTIMATE);

	fftw_execute(plan);
	fftw_destroy_plan(plan);

	if (normalize) {
		double coeff =
		    std::sqrt(std::reduce(n.begin(), n.end(), 1, std::multiplies{}));
		complex_out.transform([coeff](auto x) { return x / coeff; });
	}

	if (shift && direction == FFTW_FORWARD)
		shift_image(complex_out, true);

	using prec_t = precision_t<T>;
	if (is_real(complex_out))
		return {
		    {conversions::all_to_all<prec_t>(complex_out, false, {}, {}, {})}};
	return {{conversions::all_to_all<std::complex<prec_t>>(complex_out, false,
	                                                       {}, {}, {})}};
}

INSTANTIATE_TEMPLATE(FFT, img::FLOAT);
INSTANTIATE_TEMPLATE(FFT, img::DOUBLE);
INSTANTIATE_TEMPLATE(FFT, img::COMPLEX_F);
INSTANTIATE_TEMPLATE(FFT, img::COMPLEX_D);

} // namespace ssimp::algorithms
