#include "format_manager.hpp"
#include "../meta_types.hpp"
#include <stdexcept>

namespace fs = std::filesystem;

namespace {
template <typename format_t, typename all_types>
struct img_save_dispatcher {
	static void save(const ssimp::img::ndImageBase& img,
	                 const fs::path& path,
	                 const ssimp::OptionsManager::options_t& options) {
		throw std::logic_error("Unsupported type");
	}
};

template <typename format_t, typename type_t, typename... rest_t>
struct img_save_dispatcher<format_t, std::tuple<type_t, rest_t...>> {
	static void save(const ssimp::img::ndImageBase& img,
	                 const fs::path& path,
	                 const ssimp::OptionsManager::options_t& options) {
		if constexpr (ssimp::mt::traits::is_any_of_tuple_v<
		                  type_t, typename format_t::supported_types>) {
			if (ssimp::img::type_to_enum<type_t> == img.type()) {
				format_t::save_image(img.as_typed<type_t>(), path, options);
				return;
			}
		}

		img_save_dispatcher<format_t, std::tuple<rest_t...>>::save(img, path,
		                                                           options);
	}
};

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
		    ssimp::mt::traits::is_subset_of_v<typename first_t::supported_types,
		                                      ssimp::img::type_list>,
		    "Image format supports unknown type");

		loaders[first_t::name] = [](const fs::path& path) {
			return first_t::load_image(path);
		};
		savers[first_t::name] =
		    [](const ssimp::img::ndImageBase& img, const fs::path& path,
		       const ssimp::OptionsManager::options_t& options) {
			    img_save_dispatcher<first_t, ssimp::img::type_list>::save(
			        img, path, options);
		    };

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

std::unordered_set<std::string> FormatManager::registered_formats() const {
	std::unordered_set<std::string> formats;
	formats.reserve(_image_loaders.size());
	for (const auto& [k, _] : _image_loaders) {
		formats.insert(k);
	}

	return formats;
}
} // namespace ssimp
