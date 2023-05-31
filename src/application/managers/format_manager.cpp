#include "format_manager.hpp"

namespace fs = std::filesystem;

namespace {
template <typename T>
struct format_registerer {
	static void register_format(auto&, auto&) {}
};

template <typename first_t, typename... types_t>
struct format_registerer<std::tuple<first_t, types_t...>> {
	static void register_format(
	    std::unordered_map<
	        std::string,
	        std::function<std::vector<ssimp::img::LocalizedImage>(
	            const std::filesystem::path&)>>& loaders,
	    std::unordered_map<
	        std::string,
	        std::function<void(const ssimp::img::ndImageBase&,
	                           const std::filesystem::path&,
	                           const ssimp::OptionsManager::options_t&)>>&
	        savers) {
		static_assert(
		    mt::traits::is_subset_of_v<typename first_t::supported_types,
		                               img::type_list>,
		    "Image format supports unknown type");

		loaders[first_t::name] = auto [](const fs::path& path) {
			return first_t::load_image(path);
		};

		// TODO savers

		format_registerer<std::tuple<types_t...>>::register_format(loaders,
		                                                           savers);
	}
};
;

} // namespace

namespace ssimp {
FormatManager::FormatManager() {
	format_registerer<_registered_formats>::register_format(_image_loaders,
	                                                        _image_savers);
}

std::vector<img::LocalizedImage>
FormatManager::load_image(const fs::path& path,
                          const std::string& format) const {
	return _image_loaders.at(format)(format);
}

void FormatManager::save_image(const fs::path& directory,
                               const img::LocalizedImage& image,
                               const std::string& format,
                               const OptionsManager::options_t& options) const {
	_image_savers.at(format)(image.image, directory / image.location, options);
}
} // namespace ssimp
