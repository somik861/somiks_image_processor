#include "format_manager.hpp"
#include "../meta_types.hpp"
#include <algorithm>
#include <format>
#include <stdexcept>

namespace fs = std::filesystem;

namespace {
template <typename format_t, typename supported_types>
struct img_save_dispatcher {
	static void save(const auto& imgs, const auto& path, const auto& options) {
		throw ssimp::exceptions::Unsupported(
		    std::format("Format '{}' does not support '{}'", format_t::name,
		                ssimp::to_string(imgs[0].type())));
	}
};

template <typename format_t, typename type_t, typename... rest_t>
struct img_save_dispatcher<format_t, std::tuple<type_t, rest_t...>> {
	static void save(const auto& imgs, const auto& path, const auto& options) {
		ssimp::img::elem_type img_type = ssimp::img::type_to_enum<type_t>;
		if (imgs.size() == 0 || img_type == imgs[0].type()) {
			if (std::ranges::any_of(imgs, [=](const auto& img) {
				    return img.type() != img_type;
			    }))
				throw ssimp::exceptions::Unsupported(
				    "Saving multiple image types into single file is not "
				    "supported");

			std::vector<ssimp::img::ndImage<type_t>> typed;
			typed.reserve(imgs.size());
			for (const auto& img : imgs)
				typed.push_back(img.template as_typed<type_t>());

			format_t::save_image(typed, path, options);
			return;
		}
		img_save_dispatcher<format_t, std::tuple<rest_t...>>::save(imgs, path,
		                                                           options);
	}
};

template <typename supported_types>
struct fill_supported_types {
	static void fill(std::unordered_set<ssimp::img::elem_type>&) {}
};

template <typename type_t, typename... rest_t>
struct fill_supported_types<std::tuple<type_t, rest_t...>> {
	static void fill(std::unordered_set<ssimp::img::elem_type>& set) {
		set.insert(ssimp::img::type_to_enum<type_t>);
		fill_supported_types<std::tuple<rest_t...>>::fill(set);
	}
};

template <typename T>
struct format_registerer {
	static void register_format(auto&, auto&, auto&, auto&, auto&, auto&) {}
};

template <typename first_t, typename... types_t>
struct format_registerer<std::tuple<first_t, types_t...>> {
	static void register_format(auto& loaders,
	                            auto& savers,
	                            auto& info_getters,
	                            auto& count_verifs,
	                            auto& dims_verifs,
	                            auto& supported_types) {
		static_assert(
		    ssimp::mt::traits::is_subset_of_v<typename first_t::supported_types,
		                                      ssimp::img::type_list>,
		    "Image format supports unknown type");

		loaders[first_t::name] = [](const auto& path, const auto& options) {
			return first_t::load_image(path, options);
		};

		savers[first_t::name] = [](const auto& imgs, const auto& path,
		                           const auto& options) {
			img_save_dispatcher<
			    first_t, typename first_t::supported_types>::save(imgs, path,
			                                                      options);
		};

		info_getters[first_t::name] = [](const auto& path,
		                                 const auto& options) {
			return first_t::get_information(path, options);
		};

		count_verifs[first_t::name] = [](auto count) {
			return first_t::image_count_supported(count);
		};

		dims_verifs[first_t::name] = [](auto dims) {
			return first_t::image_dims_supported(dims);
		};

		fill_supported_types<typename first_t::supported_types>::fill(
		    supported_types[first_t::name]);

		format_registerer<std::tuple<types_t...>>::register_format(
		    loaders, savers, info_getters, count_verifs, dims_verifs,
		    supported_types);
	}
};

} // namespace

namespace ssimp {
FormatManager::FormatManager() {
	format_registerer<_registered_formats>::register_format(
	    _image_loaders, _image_savers, _information_getters, _count_verifiers,
	    _dims_verifiers, _format_supported_types);
}

std::optional<std::vector<img::LocalizedImage>>
FormatManager::load_image(const fs::path& path,
                          const std::string& format,
                          const option_types::options_t& options) const {
	return _image_loaders.at(format)(path, options);
}

void FormatManager::save_image(const fs::path& path,
                               const std::vector<img::ndImageBase>& image,
                               const std::string& format,
                               const option_types::options_t& options) const {
	_image_savers.at(format)(image, path, options);
}

std::unordered_set<std::string> FormatManager::registered_formats() const {
	std::unordered_set<std::string> formats;
	formats.reserve(_image_loaders.size());
	for (const auto& [k, _] : _image_loaders) {
		formats.insert(k);
	}

	return formats;
}

bool FormatManager::is_type_supported(const std::string& format,
                                      img::elem_type type) const {
	return _format_supported_types.at(format).contains(type);
}

bool FormatManager::is_count_supported(const std::string& format,
                                       std::size_t count) const {
	return _count_verifiers.at(format)(count);
}

bool FormatManager::is_dims_supported(const std::string& format,
                                      std::span<const std::size_t> dims) const {
	return _dims_verifiers.at(format)(dims);
}

std::optional<ssimp::ImageProperties> FormatManager::get_image_information(
    const std::filesystem::path& path,
    const std::string& format,
    const option_types::options_t options) const {
	return _information_getters.at(format)(path, options);
}
} // namespace ssimp
