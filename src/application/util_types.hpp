#pragma once

#include <ostream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace ssimp {
namespace option_types {
/**
 * Variant of possible option values
 */
using value_t = std::variant<bool, int32_t, double, std::string>;
/**
 * Type representing a set of options
 */
using options_t = std::unordered_map<std::string, value_t>;
} // namespace option_types

class ImageProperties {
  public:
	std::string format;
	std::vector<std::size_t> dims;
	std::unordered_map<std::string, std::string> others;

	friend std::ostream& operator<<(std::ostream& os,
	                                const ImageProperties& imgprop) {
		os << "Format: " << imgprop.format << '\n';
		os << "Dims: [ ";
		for (std::size_t dim : imgprop.dims)
			os << dim << " ";
		os << "]\n";
		for (const auto& [k, v] : imgprop.others)
			os << k << ": " << v;
		os << '\n';
		return os;
	}
};
} // namespace ssimp
