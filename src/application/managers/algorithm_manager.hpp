#pragma once

#include "../../algorithms/split_channels.hpp"
#include "../nd_image.hpp"
#include <functional>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

namespace ssimp {
class AlgorithmManager {
  private:
	using _registered_algorithms = std::tuple<algorithms::SplitChannels>;

  public:
	/**
	 * Modified constructor for algorithm manager initialization.
	 */
	AlgorithmManager();

	/**
	 * Apply **algorithm** to images.
	 */
	std::vector<img::LocalizedImage>
	apply(const std::vector<img::ndImageBase>& images,
	      const std::string& algorithm,
	      const option_types::options_t& options) const;

	/**
	 * Return true if type is supported.
	 */
	bool is_type_supported(const std::string& algorithm,
	                       img::elem_type type) const;

	/**
	 * Return true if image count is supported
	 */
	bool is_count_supported(const std::string& algorithm,
	                        std::size_t count) const;

	/**
	 * Return true if image dimensionality is supported
	 */
	bool is_dims_supported(const std::string& algorithm,
	                       std::span<const std::size_t> dims) const;

	/**
	 * Return true if same dims of multiple input images are required
	 */
	bool is_same_dims_required(const std::string& algorithm) const;

	/**
	 * Get names of registered algorithms
	 */
	std::unordered_set<std::string> registered_algorithms() const;

  private:
	using _options_t = option_types::options_t;
	template <typename fun_t>
	using _funmap_t = std::unordered_map<std::string, fun_t>;

	using _count_verifier_t = std::function<bool(std::size_t)>;
	using _dims_verifier_t = std::function<bool(std::span<const std::size_t>)>;
	using _applier_t = std::function<std::vector<img::LocalizedImage>(
	    const std::vector<img::ndImageBase>&, const option_types::options_t&)>;

	_funmap_t<_count_verifier_t> _count_verifiers;
	_funmap_t<_dims_verifier_t> _dims_verifiers;
	_funmap_t<_applier_t> _appliers;

	std::unordered_map<std::string, std::unordered_set<img::elem_type>>
	    _supported_types;
	std::unordered_map<std::string, bool> _same_dims_required;
};
} // namespace ssimp
