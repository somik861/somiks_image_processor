#pragma once
#include "common.hpp"
#include <cmath>
#include <limits>
#include <numeric>

namespace ssimp::algorithms::conversions {

using gray_types =
    std::tuple<img::GRAY8, img::GRAY16, img::GRAY32, img::GRAY64>;
using float_types = std::tuple<img::FLOAT, img::DOUBLE>;
using complex_types = std::tuple<img::COMPLEX_F, img::COMPLEX_D>;

// ========== TO GRAY ===========
template <typename out_t, typename in_t>
img::ndImage<out_t> gray_to_gray(const img::ndImage<in_t>& img_, bool rescale) {
	if constexpr (std::is_same_v<out_t, in_t>)
		return img_;

	img::ndImage<out_t> out(img_.dims());

	std::ranges::transform(img_, out.begin(), [=](auto elem) {
		if (!rescale)
			return out_t(elem);

		if constexpr (sizeof(in_t) < sizeof(out_t))
			return out_t(out_t(elem) << (sizeof(out_t) * 8 - sizeof(in_t) * 8));
		else
			return out_t(elem >> (sizeof(in_t) * 8 - sizeof(out_t) * 8));
	});

	return out;
}

template <typename out_t, typename in_t>
img::ndImage<out_t> float_to_gray(const img::ndImage<in_t>& img_,
                                  bool rescale) {
	img::ndImage<out_t> out(img_.dims());

	std::ranges::transform(img_, out.begin(), [=](auto elem) {
		if (!rescale)
			return out_t(elem);
		return out_t(std::llround(elem * std::numeric_limits<out_t>::max()));
	});

	return out;
}

template <typename out_t>
img::ndImage<out_t> ga_to_gray(const img::ndImage<img::GRAY8A>& img_,
                               bool rescale,
                               img::GRAY8 gray_bg) {
	img::ndImage<img::GRAY8> out(img_.dims());
	std::ranges::transform(img_, out.begin(), [=](auto elem) {
		return img::GRAY8(std::lround(elem[0] * (elem[1] / 255.0) +
		                              gray_bg * (1 - elem[1] / 255.0)));
	});

	return gray_to_gray<out_t>(out, rescale);
}

template <typename out_t>
img::ndImage<out_t> rgb_to_gray(const img::ndImage<img::RGB8>& img_,
                                bool rescale,
                                std::array<double, 3> rgb_multipliers) {
	img::ndImage<img::GRAY8> out(img_.dims());
	std::ranges::transform(img_, out.begin(), [=](auto elem) {
		return img::GRAY8(std::lround(std::inner_product(
		    elem.begin(), elem.end(), rgb_multipliers.begin(), 0.0)));
	});

	return gray_to_gray<out_t>(out, rescale);
}

inline img::ndImage<img::RGB8> rgba_to_rgb(const img::ndImage<img::RGBA8>& img_,
                                           img::RGB8 bg);
template <typename out_t>
img::ndImage<out_t> rgba_to_gray(const img::ndImage<img::RGBA8>& img_,
                                 bool rescale,
                                 std::array<double, 3> rgb_multipliers,
                                 img::RGB8 bg) {
	return rgb_to_gray<out_t>(rgba_to_rgb(img_, bg), rescale, rgb_multipliers);
}

// ========== TO FLOAT ===========

template <typename out_t, typename in_t>
img::ndImage<out_t> gray_to_float(const img::ndImage<in_t>& img_,
                                  bool rescale) {
	img::ndImage<out_t> out(img_.dims());

	std::ranges::transform(img_, out.begin(), [=](auto elem) {
		if (!rescale)
			return out_t(elem);
		return out_t(elem) / std::numeric_limits<in_t>::max();
	});

	return out;
}

template <typename out_t, typename in_t>
img::ndImage<out_t> float_to_float(const img::ndImage<in_t>& img_) {
	if constexpr (std::is_same_v<out_t, in_t>)
		return img_;
	img::ndImage<out_t> out(img_.dims());

	std::ranges::transform(img_, out.begin(),
	                       [=](auto elem) { return out_t(elem); });

	return out;
}

template <typename out_t>
img::ndImage<out_t> ga_to_float(const img::ndImage<img::GRAY8A>& img_,
                                bool rescale,
                                img::GRAY8 gray_bg) {
	return gray_to_float<out_t>(ga_to_gray<img::GRAY8>(img_, false, gray_bg),
	                            rescale);
}

template <typename out_t>
img::ndImage<out_t> rgb_to_float(const img::ndImage<img::RGB8>& img_,
                                 bool rescale,
                                 std::array<double, 3> rgb_multipliers) {
	return gray_to_float<out_t>(
	    rgb_to_gray<img::GRAY8>(img_, false, rgb_multipliers), rescale);
}

template <typename out_t>
img::ndImage<out_t> rgba_to_float(const img::ndImage<img::RGBA8>& img_,
                                  bool rescale,
                                  std::array<double, 3> rgb_multipliers,
                                  img::RGB8 bg) {
	return gray_to_float<out_t>(
	    rgba_to_gray<img::GRAY8>(img_, false, rgb_multipliers, bg), rescale);
}

// ========== TO GA ===========
template <typename in_t>
img::ndImage<img::GRAY8A> gray_to_ga(const img::ndImage<in_t>& img_,
                                     bool rescale) {
	auto gray8 = gray_to_gray<img::GRAY8>(img_, rescale);

	img::ndImage<img::GRAY8A> out(img_.dims());
	std::ranges::transform(gray8, out.begin(), [](auto elem) {
		return img::GRAY8A{elem, 255};
	});

	return out;
}

template <typename in_t>
img::ndImage<img::GRAY8A> float_to_ga(const img::ndImage<in_t>& img_,
                                      bool rescale) {
	return gray_to_ga(float_to_gray<img::GRAY8>(img_, rescale), rescale);
}

inline img::ndImage<img::GRAY8A>
rgb_to_ga(const img::ndImage<img::RGB8>& img_,
          std::array<double, 3> rgb_multipliers) {
	return gray_to_ga(rgb_to_gray<img::GRAY8>(img_, false, rgb_multipliers),
	                  false);
}

inline img::ndImage<img::GRAY8A>
rgba_to_ga(const img::ndImage<img::RGBA8>& img_,
           std::array<double, 3> rgb_multipliers) {
	img::ndImage<img::GRAY8A> out(img_.dims());

	std::ranges::transform(img_, out.begin(), [&](auto elem) {
		return img::GRAY8A{
		    img::GRAY8(std::lround(std::inner_product(
		        elem.begin(), elem.end(), rgb_multipliers.begin(), 0.0))),
		    elem[3]};
	});

	return out;
}

// ========== TO RGB ===========
template <typename in_t>
img::ndImage<img::RGB8> gray_to_rgb(const img::ndImage<in_t>& img_,
                                    bool rescale) {
	img::ndImage<img::RGB8> out(img_.dims());

	std::ranges::transform(gray_to_gray<img::GRAY8>(img_, rescale), out.begin(),
	                       [](auto elem) {
		                       return img::RGB8{elem, elem, elem};
	                       });

	return out;
}

template <typename in_t>
img::ndImage<img::RGB8> float_to_rgb(const img::ndImage<in_t>& img_,
                                     bool rescale) {
	img::ndImage<img::RGB8> out(img_.dims());

	std::ranges::transform(float_to_gray<img::GRAY8>(img_, rescale),
	                       out.begin(), [](auto elem) {
		                       return img::RGB8{elem, elem, elem};
	                       });

	return out;
}

inline img::ndImage<img::RGB8> ga_to_rgb(const img::ndImage<img::GRAY8A>& img_,
                                         bool rescale,
                                         img::GRAY8 gray_bg) {
	return gray_to_rgb(ga_to_gray<img::GRAY8>(img_, rescale, gray_bg), rescale);
}
/* inline */ img::ndImage<img::RGB8>
rgba_to_rgb(const img::ndImage<img::RGBA8>& img_, img::RGB8 bg) {
	img::ndImage<img::RGB8> out(img_.dims());

	std::ranges::transform(img_, out.begin(), [=](auto elem) {
		return img::RGB8{
		    img::GRAY8(std::lround(elem[0] * (elem[3] / 255.0) +
		                           bg[0] * (1 - (elem[3] / 255.0)))),
		    img::GRAY8(std::lround(elem[1] * (elem[3] / 255.0) +
		                           bg[1] * (1 - (elem[3] / 255.0)))),
		    img::GRAY8(std::lround(elem[2] * (elem[3] / 255.0) +
		                           bg[2] * (1 - (elem[3] / 255.0))))};
	});

	return out;
}

// ========== TO RGBA ===========
inline img::ndImage<img::RGBA8>
rgb_to_rgba(const img::ndImage<img::RGB8>& img_);

template <typename in_t>
img::ndImage<img::RGBA8> gray_to_rgba(const img::ndImage<in_t>& img_,
                                      bool rescale) {
	return rgb_to_rgba(gray_to_rgb(img_, rescale));
}

template <typename in_t>
img::ndImage<img::RGBA8> float_to_rgba(const img::ndImage<in_t>& img_,
                                       bool rescale) {
	return rgb_to_rgba(
	    gray_to_rgb(float_to_gray<img::GRAY8>(img_, rescale), false));
}

inline img::ndImage<img::RGBA8>
ga_to_rgba(const img::ndImage<img::GRAY8A>& img_) {
	img::ndImage<img::RGBA8> out(img_.dims());

	std::ranges::transform(img_, out.begin(), [](auto elem) {
		return img::RGBA8{elem[0], elem[0], elem[0], elem[1]};
	});

	return out;
}

/* inline */ img::ndImage<img::RGBA8>
rgb_to_rgba(const img::ndImage<img::RGB8>& img_) {
	img::ndImage<img::RGBA8> out(img_.dims());

	std::ranges::transform(img_, out.begin(), [](auto elem) {
		return img::RGBA8{elem[0], elem[1], elem[2], 255};
	});

	return out;
}

// ========== COMPLEX ===========
template <typename out_t, typename in_t>
img::ndImage<out_t> all_to_all(const img::ndImage<in_t>& img_,
                               bool rescale,
                               std::array<double, 3> rgb_multipliers,
                               img::GRAY8 gray_bg,
                               img::RGB8 rgb_bg);

template <typename out_t, typename in_t>
img::ndImage<out_t> noncomplex_to_complex(const img::ndImage<in_t>& img_,
                                          bool rescale,
                                          std::array<double, 3> rgb_multipliers,
                                          img::GRAY8 gray_bg,
                                          img::RGB8 rgb_bg) {
	using prec_t = typename out_t::value_type;
	img::ndImage<prec_t> float_img =
	    all_to_all<prec_t>(img_, rescale, rgb_multipliers, gray_bg, rgb_bg);

	img::ndImage<std::complex<prec_t>> out(img_.dims());
	std::ranges::transform(float_img, out.begin(), [](auto elem) {
		return std::complex<prec_t>(elem, 0);
	});

	return out;
}

template <typename out_t, typename in_t>
img::ndImage<out_t> complex_to_complex(const img::ndImage<in_t>& img_) {

	using inprec_t = typename in_t::value_type;
	using prec_t = typename out_t::value_type;

	if constexpr (std::is_same_v<inprec_t, prec_t>)
		return img_;

	img::ndImage<std::complex<prec_t>> out(img_.dims());
	std::ranges::transform(img_, out.begin(), [](auto elem) {
		return std::complex<prec_t>(elem.real(), elem.imag());
	});

	return out;
}

template <typename out_t, typename in_t>
img::ndImage<out_t> complex_to_noncomplex(const img::ndImage<in_t>& img_,
                                          bool rescale,
                                          std::array<double, 3> rgb_multipliers,
                                          img::GRAY8 gray_bg,
                                          img::RGB8 rgb_bg) {

	using inprec_t = typename in_t::value_type;

	img::ndImage<inprec_t> float_img(img_.dims());
	std::ranges::transform(img_, float_img.begin(),
	                       [](auto elem) { return elem.real(); });

	return all_to_all<out_t>(float_img, rescale, rgb_multipliers, gray_bg,
	                         rgb_bg);
}

// ========== DISPATCHERS ===========
// all to gray
template <typename out_t, typename in_t>
    requires mt::traits::is_any_of_tuple_v<out_t, gray_types>
img::ndImage<out_t> all_to_all(const img::ndImage<in_t>& img_,
                               bool rescale,
                               std::array<double, 3> rgb_multipliers,
                               img::GRAY8 gray_bg,
                               img::RGB8 rgb_bg) {
	if constexpr (std::is_same_v<in_t, out_t>)
		return img_;

	if constexpr (mt::traits::is_any_of_tuple_v<in_t, gray_types>)
		return gray_to_gray<out_t>(img_, rescale);
	else if constexpr (mt::traits::is_any_of_tuple_v<in_t, float_types>)
		return float_to_gray<out_t>(img_, rescale);
	else if constexpr (mt::traits::is_any_of_tuple_v<in_t, complex_types>)
		return complex_to_noncomplex<out_t>(img_, rescale, rgb_multipliers,
		                                    gray_bg, rgb_bg);
	else if constexpr (std::is_same_v<in_t, img::GRAY8A>)
		return ga_to_gray<out_t>(img_, rescale, gray_bg);
	else if constexpr (std::is_same_v<in_t, img::RGB8>)
		return rgb_to_gray<out_t>(img_, rescale, rgb_multipliers);
	else if constexpr (std::is_same_v<in_t, img::RGBA8>)
		return rgba_to_gray<out_t>(img_, rescale, rgb_multipliers, rgb_bg);

	throw exceptions::Unsupported("Unsupported conversion");
}

// all to float
template <typename out_t, typename in_t>
    requires mt::traits::is_any_of_tuple_v<out_t, float_types>
img::ndImage<out_t> all_to_all(const img::ndImage<in_t>& img_,
                               bool rescale,
                               std::array<double, 3> rgb_multipliers,
                               img::GRAY8 gray_bg,
                               img::RGB8 rgb_bg) {
	if constexpr (std::is_same_v<in_t, out_t>)
		return img_;

	if constexpr (mt::traits::is_any_of_tuple_v<in_t, gray_types>)
		return gray_to_float<out_t>(img_, rescale);
	else if constexpr (mt::traits::is_any_of_tuple_v<in_t, float_types>)
		return float_to_float<out_t>(img_);
	else if constexpr (mt::traits::is_any_of_tuple_v<in_t, complex_types>)
		return complex_to_noncomplex<out_t>(img_, rescale, rgb_multipliers,
		                                    gray_bg, rgb_bg);
	else if constexpr (std::is_same_v<in_t, img::GRAY8A>)
		return ga_to_float<out_t>(img_, rescale, gray_bg);
	else if constexpr (std::is_same_v<in_t, img::RGB8>)
		return rgb_to_float<out_t>(img_, rescale, rgb_multipliers);
	else if constexpr (std::is_same_v<in_t, img::RGBA8>)
		return rgba_to_float<out_t>(img_, rescale, rgb_multipliers, rgb_bg);

	throw exceptions::Unsupported("Unsupported conversion");
}

// all to ga
template <typename out_t, typename in_t>
    requires std::is_same_v<out_t, img::GRAY8A>
img::ndImage<out_t> all_to_all(const img::ndImage<in_t>& img_,
                               bool rescale,
                               std::array<double, 3> rgb_multipliers,
                               img::GRAY8 gray_bg,
                               img::RGB8 rgb_bg) {
	if constexpr (std::is_same_v<in_t, out_t>)
		return img_;

	if constexpr (mt::traits::is_any_of_tuple_v<in_t, gray_types>)
		return gray_to_ga(img_, rescale);
	else if constexpr (mt::traits::is_any_of_tuple_v<in_t, float_types>)
		return float_to_ga(img_, rescale);
	else if constexpr (mt::traits::is_any_of_tuple_v<in_t, complex_types>)
		return complex_to_noncomplex<out_t>(img_, rescale, rgb_multipliers,
		                                    gray_bg, rgb_bg);
	else if constexpr (std::is_same_v<in_t, img::RGB8>)
		return rgb_to_ga(img_, rgb_multipliers);
	else if constexpr (std::is_same_v<in_t, img::RGBA8>)
		return rgba_to_ga(img_, rgb_multipliers);

	throw exceptions::Unsupported("Unsupported conversion");
}

// all to rgb
template <typename out_t, typename in_t>
    requires std::is_same_v<out_t, img::RGB8>
img::ndImage<out_t> all_to_all(const img::ndImage<in_t>& img_,
                               bool rescale,
                               std::array<double, 3> rgb_multipliers,
                               img::GRAY8 gray_bg,
                               img::RGB8 rgb_bg) {
	if constexpr (std::is_same_v<in_t, out_t>)
		return img_;

	if constexpr (mt::traits::is_any_of_tuple_v<in_t, gray_types>)
		return gray_to_rgb(img_, rescale);
	else if constexpr (mt::traits::is_any_of_tuple_v<in_t, float_types>)
		return float_to_rgb(img_, rescale);
	else if constexpr (mt::traits::is_any_of_tuple_v<in_t, complex_types>)
		return complex_to_noncomplex<out_t>(img_, rescale, rgb_multipliers,
		                                    gray_bg, rgb_bg);
	else if constexpr (std::is_same_v<in_t, img::GRAY8A>)
		return ga_to_rgb(img_, rescale, gray_bg);
	else if constexpr (std::is_same_v<in_t, img::RGBA8>)
		return rgba_to_rgb(img_, rgb_bg);

	throw exceptions::Unsupported("Unsupported conversion");
}

// all to rgba
template <typename out_t, typename in_t>
    requires std::is_same_v<out_t, img::RGBA8>
img::ndImage<out_t> all_to_all(const img::ndImage<in_t>& img_,
                               bool rescale,
                               std::array<double, 3> rgb_multipliers,
                               img::GRAY8 gray_bg,
                               img::RGB8 rgb_bg) {
	if constexpr (std::is_same_v<in_t, out_t>)
		return img_;

	if constexpr (mt::traits::is_any_of_tuple_v<in_t, gray_types>)
		return gray_to_rgba(img_, rescale);
	else if constexpr (mt::traits::is_any_of_tuple_v<in_t, float_types>)
		return float_to_rgba(img_, rescale);
	else if constexpr (mt::traits::is_any_of_tuple_v<in_t, complex_types>)
		return complex_to_noncomplex<out_t>(img_, rescale, rgb_multipliers,
		                                    gray_bg, rgb_bg);
	else if constexpr (std::is_same_v<in_t, img::GRAY8A>)
		return ga_to_rgba(img_);
	else if constexpr (std::is_same_v<in_t, img::RGB8>)
		return rgb_to_rgba(img_);

	throw exceptions::Unsupported("Unsupported conversion");
}

// all to complex
template <typename out_t, typename in_t>
    requires mt::traits::is_any_of_tuple_v<out_t, complex_types>
img::ndImage<out_t> all_to_all(const img::ndImage<in_t>& img_,
                               bool rescale,
                               std::array<double, 3> rgb_multipliers,
                               img::GRAY8 gray_bg,
                               img::RGB8 rgb_bg) {
	if constexpr (std::is_same_v<in_t, out_t>)
		return img_;

	if constexpr (mt::traits::is_any_of_tuple_v<in_t, complex_types>)
		return complex_to_complex<out_t>(img_);
	return noncomplex_to_complex<out_t>(img_, rescale, rgb_multipliers, gray_bg,
	                                    rgb_bg);
}

} // namespace ssimp::algorithms::conversions
