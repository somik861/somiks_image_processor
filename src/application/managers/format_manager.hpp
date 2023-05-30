#pragma once

#include "../nd_image.hpp"
#include "options_manager.hpp"
#include <filesystem>
#include <tuple>

namespace ssimp {
class FormatManager {
  private:
	using registered_types = std::tuple<>;

  public:
	void initialize();
	img::LocalizedImage load_image(const std::filesystem::path& path) const;
	void save_image(const std::filesystem::path& directory,
	                const img::LocalizedImage& image,
	                const OptionsManager::options_t& options) const;
};
} // namespace ssimp
