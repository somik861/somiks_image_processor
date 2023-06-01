#include "testing_sample.hpp"
#include "common.hpp"

namespace ssimp::formats {
/* static */ std::vector<img::LocalizedImage>
TestingSample::load_image(const fs::path& path) {

	return {{img::ndImage<img::GRAY8>(2, 2), path.filename()}};
}

template <typename T>
    requires mt::traits::is_any_of_tuple_v<T, TestingSample::supported_types>
/* static */ void
TestingSample::save_image(const img::ndImage<T>& img,
                          const fs::path& path,
                          const option_types::options_t& options) {}

INSTANTIATE_SAVE_TEMPLATE(TestingSample, img::GRAY8);

} // namespace ssimp::formats
