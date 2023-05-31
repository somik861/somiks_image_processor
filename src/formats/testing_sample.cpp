#include "testing_sample.hpp"

namespace ssimp::formats {
/* static */ std::vector<img::LocalizedImage>
TestingSample::load_image(const std::filesystem::path&) {
	return {};
}

template <typename T>
    requires mt::traits::is_type_of_tuple_v<T, TestingSample::supported_types>
/* static */ void
TestingSample::save_image(const img::ndImage<T>& img,
                          const std::filesystem::path& path,
                          const OptionsManager::options_t& options) {}
} // namespace ssimp::formats
