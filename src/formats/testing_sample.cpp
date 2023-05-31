#include "testing_sample.hpp"
#include "common.hpp"

namespace fs = std::filesystem;

namespace ssimp::formats {
template class generate_templates<TestingSample>;

/* static */ std::vector<img::LocalizedImage>
TestingSample::load_image(const std::filesystem::path&) {
	return {};
}

template <typename T>
    requires mt::traits::is_any_of_tuple_v<T, TestingSample::supported_types>
/* static */ void
TestingSample::save_image(const img::ndImage<T>& img,
                          const fs::path& path,
                          const OptionsManager::options_t& options) {}
} // namespace ssimp::formats
