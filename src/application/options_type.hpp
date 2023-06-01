#pragma once

#include <string>
#include <unordered_map>
#include <variant>

namespace ssimp::option_types {
/**
 * Variant of possible option values
 */
using value_t = std::variant<bool, int32_t, double, std::string>;
/**
 * Type representing a set of options
 */
using options_t = std::unordered_map<std::string, value_t>;
} // namespace ssimp::option_types
