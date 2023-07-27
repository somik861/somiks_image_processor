#include "fft.hpp"
#include "common_macro.hpp"

namespace ssimp::algorithms {
bool FFT::image_count_supported(std::size_t count) { return count == 1; }
bool FFT::image_dims_supported(std::span<const std::size_t> dims) {
	return dims.size() > 0 &&
	       std::ranges::all_of(dims, [](auto x) { return x > 0; });
}
bool FFT::same_dims_required() { return true; }

template <typename T>
    requires mt::traits::is_any_of_tuple_v<T, FFT::supported_types>
static std::vector<img::LocalizedImage>
FFT::apply(const std::vector<img::ndImage<T>>& imgs,
           const option_types::options_t& options) {
	return {};
}

} // namespace ssimp::algorithms
