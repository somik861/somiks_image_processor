#include "testing_sample.hpp"
#include "common_macro.hpp"

namespace ssimp::formats {
/* static */ std::optional<std::vector<img::LocalizedImage>>
TestingSample::load_image(const fs::path& path,
                          const option_types::options_t&) {
	return std::vector<img::LocalizedImage>{};
}

/* static */
std::optional<ImageProperties>
TestingSample::get_information(const fs::path& path,
                               const option_types::options_t&) {
	return ImageProperties{name, {}, {}};
}

/* static */
bool TestingSample::image_count_supported(std::size_t count) { return true; }

/* static */
bool TestingSample::image_dims_supported(std::span<const std::size_t> dims) {
	return true;
}

template <typename T>
    requires mt::traits::is_any_of_tuple_v<T, TestingSample::supported_types>
/* static */ void
TestingSample::save_image(const std::vector<img::ndImage<T>>& imgs,
                          const fs::path& path,
                          const option_types::options_t& options) {
	std::ofstream f(path);
	f << std::format("Writing {} images to '{}'\n\n", imgs.size(),
	                 ssimp::to_string(path));

	for (const auto& img : imgs) {
		f << img << '\n';
	}
}

INSTANTIATE_SAVE_TEMPLATE(TestingSample, img::GRAY8);

} // namespace ssimp::formats
