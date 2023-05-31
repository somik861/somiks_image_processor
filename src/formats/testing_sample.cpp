#include "testing_sample.hpp"

namespace fs = std::filesystem;

namespace ssimp::formats {
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

/*
template void
TestingSample::save_image<img::GRAY8>(const img::ndImage<img::GRAY8>&,
                                      const fs::path&,
                                      const OptionsManager::options_t&);
                                      */
} // namespace ssimp::formats
