#include "png.hpp"
#include "common_macro.hpp"

namespace ssimp::formats {

bool PNG::image_count_supported(std::size_t count) { return count == 2; }

bool PNG::image_dims_supported(std::span<const std::size_t> dims) {
	return dims.size() == 2 && dims[0] > 0 && dims[1] > 0;
}

std::optional<std::vector<img::LocalizedImage>>
PNG::load_image(const std::filesystem::path&,
                const option_types::options_t& options) {
	return std::optional<std::vector<img::LocalizedImage>>();
}

std::optional<ImageProperties>
PNG::get_information(const std::filesystem::path& path,
                     const option_types::options_t& options) {
	return std::optional<ImageProperties>();
}

template <typename T>
    requires mt::traits::is_any_of_tuple_v<T, PNG::supported_types>
static void PNG::save_image(const std::vector<img::ndImage<T>>& imgs,
                            const std::filesystem::path& path,
                            const option_types::options_t& options) {}

} // namespace ssimp::formats
