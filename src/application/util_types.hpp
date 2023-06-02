#pragma once

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace ssimp {
namespace exceptions {
class Exception : public std::exception {
  public:
	Exception(std::string what_arg) : _what(std::move(what_arg)) {}
	virtual const char* what() const noexcept { return _what.data(); }

  private:
	std::string _what;
};

class IOError : public Exception {
  public:
	using Exception::Exception;
};

class Unsupported : public Exception {
  public:
	using Exception::Exception;
};

} // namespace exceptions
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
};
} // namespace ssimp
