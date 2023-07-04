#include "algorithm_manager.hpp"
#include <format>

namespace {
template <typename algorithm_t, typename supported_types>
struct img_dispatcher {
	static std::vector<ssimp::img::LocalizedImage> apply(const auto& imgs,
	                                                     const auto& options) {
		throw ssimp::exceptions::Unsupported(
		    std::format("Algorithm '{}' does not support '{}'",
		                algorithm_t::name, ssimp::to_string(imgs[0].type())));
	}
};

template <typename algorithm_t, typename type_t, typename... rest_t>
struct img_dispatcher<algorithm_t, std::tuple<type_t, rest_t...>> {
	static std::vector<ssimp::img::LocalizedImage> apply(const auto& imgs,
	                                                     const auto& options) {
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

			return algorithm_t::apply(typed, options);
		}
		return img_dispatcher<algorithm_t, std::tuple<rest_t...>>::apply(
		    imgs, options);
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
struct algorithm_registerer {
	static void register_algorithm(auto&, auto&, auto&, auto&, auto&) {}
};

template <typename first_t, typename... types_t>
struct algorithm_registerer<std::tuple<first_t, types_t...>> {
	static void register_algorithm(auto& appliers,
	                               auto& count_verifs,
	                               auto& dims_verifs,
	                               auto& same_dims,
	                               auto& supported_types) {
		static_assert(
		    ssimp::mt::traits::is_subset_of_v<typename first_t::supported_types,
		                                      ssimp::img::type_list>,
		    "Algorithm supports unknown type");

		appliers[first_t::name] = [](const auto& images, const auto& options) {
			return img_dispatcher<
			    first_t, typename first_t::supported_types>::apply(images,
			                                                       options);
		};

		count_verifs[first_t::name] = [](auto count) {
			return first_t::image_count_supported(count);
		};

		dims_verifs[first_t::name] = [](auto dims) {
			return first_t::image_dims_supported(dims);
		};

		same_dims[first_t::name] = first_t::same_dims_required();

		fill_supported_types<typename first_t::supported_types>::fill(
		    supported_types[first_t::name]);

		algorithm_registerer<std::tuple<types_t...>>::register_algorithm(
		    appliers, count_verifs, dims_verifs, same_dims, supported_types);
	}
};
} // namespace

namespace ssimp {
AlgorithmManager::AlgorithmManager() {
	algorithm_registerer<_registered_algorithms>::register_algorithm(
	    _appliers, _count_verifiers, _dims_verifiers, _same_dims_required,
	    _supported_types);
};

std::unordered_set<std::string>
AlgorithmManager::registered_algorithms() const {
	std::unordered_set<std::string> algorithms;
	algorithms.reserve(_appliers.size());

	for (const auto& [k, _] : _appliers) {
		algorithms.insert(k);
	}

	return algorithms;
}

bool AlgorithmManager::is_type_supported(const std::string& algorithm,
                                         img::elem_type type) const {
	return _supported_types.at(algorithm).contains(type);
}

bool AlgorithmManager::is_count_supported(const std::string& algorithm,
                                          std::size_t count) const {
	return _count_verifiers.at(algorithm)(count);
}

bool AlgorithmManager::is_dims_supported(
    const std::string& algorithm, std::span<const std::size_t> dims) const {
	return _dims_verifiers.at(algorithm)(dims);
}

bool AlgorithmManager::is_same_dims_required(
    const std::string& algorithm) const {
	return _same_dims_required.at(algorithm);
}

std::vector<img::LocalizedImage>
AlgorithmManager::apply(const std::vector<img::ndImageBase>& images,
                        const std::string& algorithm,
                        const option_types::options_t& options) const {
	return _appliers.at(algorithm)(images, options);
}

} // namespace ssimp
