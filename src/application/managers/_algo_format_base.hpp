#pragma once

#include "../nd_image.hpp"
#include "../utils.hpp"
#include <functional>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace ssimp::details {
class _AlgoFormatBase {
  public:
	/**
	 * Return true if type is supported.
	 */
	bool is_type_supported(const std::string& element,
	                       img::elem_type type) const;

	/**
	 * Return set of supported types.
	 */
	const std::unordered_set<img::elem_type>&
	supported_types(const std::string& element) const;

	/**
	 * Return true if image count is supported
	 */
	bool is_count_supported(const std::string& element,
	                        std::size_t count) const;

	/**
	 * Return true if image dimensionality is supported
	 */
	bool is_dims_supported(const std::string& element,
	                       std::span<const std::size_t> dims) const;

	/**
	 * Return true if same dims of multiple input images are required
	 */
	bool is_same_dims_required(const std::string& element) const;

	/**
	 * Get names of registered elements
	 */
	std::unordered_set<std::string> registered() const;

	/**
	 * Return true if element is registered. Is much faster then obtatining all
	 * the registered elemets in for loop.
	 */
	bool is_registered(const std::string& element) const;

  protected:
	using _options_t = option_types::options_t;
	template <typename fun_t>
	using _funmap_t = std::unordered_map<std::string, fun_t>;

	using _count_verifier_t = std::function<bool(std::size_t)>;
	using _dims_verifier_t = std::function<bool(std::span<const std::size_t>)>;

	_funmap_t<_count_verifier_t> _count_verifiers;
	_funmap_t<_dims_verifier_t> _dims_verifiers;

	std::unordered_map<std::string, std::unordered_set<img::elem_type>>
	    _supported_types;
	std::unordered_map<std::string, bool> _same_dims_required;
};
} // namespace ssimp::details
