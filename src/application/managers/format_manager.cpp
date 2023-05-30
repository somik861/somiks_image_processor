#include "format_manager.hpp"

namespace fs = std::filesystem;

namespace ssimp {
FormatManager::FormatManager() {}

std::vector<img::LocalizedImage>
FormatManager::load_image(const fs::path& path) const {
	return {};
}

void FormatManager::save_image(const fs::path& directory,
                               const img::LocalizedImage& image,
                               const std::string& format,
                               const OptionsManager::options_t& options) const {
}
} // namespace ssimp
