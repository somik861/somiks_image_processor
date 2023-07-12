#pragma once

#include "meta_types.hpp"
#include <array>
#include <cassert>
#include <format>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace ssimp {
namespace details {
class IosStateHolder {
  public:
	IosStateHolder(std::ostream& ios) : _ios(ios), _state(ios.rdstate()) {}
	~IosStateHolder() { _ios.setstate(_state); }

  private:
	std::ostream& _ios;
	std::ios::iostate _state;
};

inline std::string indent(const std::string& str) {
	std::string out;
	for (char ch : str) {
		out += ch;
		if (ch == '\n')
			out += '\t';
	}
	out.pop_back();
	return out;
}
} // namespace details

template <typename T>
std::string to_string(const T& arg) {
	std::stringstream ss;
	ss << arg;
	return ss.str();
}

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

struct no_value {};

class OptionConfig {
  public:
	using value_t = mt::traits::variant_prepend_t<option_types::no_value,
	                                              option_types::value_t>;

	std::string& id() { return _id; }
	const std::string& id() const { return _id; }
	std::string& type() { return _type; }
	const std::string& type() const { return _type; }
	std::string& text() { return _text; }
	const std::string& text() const { return _text; }
	value_t& default_() { return _default; }
	const value_t& default_() const { return _default; }
	const std::vector<std::string>& values() const {
		assert(_type == "choice");
		return _values;
	}
	std::vector<std::string>& values() {
		assert(_type == "choice");
		return _values;
	}
	std::array<value_t, 2>& range() { return _range; }
	const std::array<value_t, 2>& range() const { return _range; }
	std::vector<OptionConfig>& options() {
		assert(_type == "subsection");
		return _options;
	}
	const std::vector<OptionConfig>& options() const {
		assert(_type == "subsection");
		return _options;
	}

	template <typename T>
	    requires mt::concepts::
	        TupleType<T, mt::traits::variant_to_tuple_t<option_types::value_t>>
	    const T& default_as() const {
		return std::get<T>(_default);
	}

	template <typename T>
	    requires mt::concepts::
	        TupleType<T, mt::traits::variant_to_tuple_t<option_types::value_t>>
	    std::array<T, 2> range_as() const {
		return {std::get<T>(_range[0]), std::get<T>(_range[1])};
	}

  private:
	std::string _id;
	std::string _type;
	std::string _text;
	value_t _default;
	std::vector<std::string> _values;
	std::array<value_t, 2> _range;
	std::vector<OptionConfig> _options;
};

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
			os << k << ": " << v << '\n';
		return os;
	}
};
} // namespace ssimp

inline std::ostream& operator<<(std::ostream& os,
                                const ssimp::option_types::options_t& options) {
	os << "Options:\n";
	for (const auto& [k, v] : options) {
		os << "    " << k << ": ";
		std::visit([&](auto x) { os << std::boolalpha << x; }, v);
		os << '\n';
	}
	return os;
}

inline std::ostream& operator<<(std::ostream& os,
                                const ssimp::option_types::no_value& val) {
	throw std::runtime_error("'no_value' can not be streamed");
}

inline std::ostream& operator<<(std::ostream& os,
                                const ssimp::option_types::OptionConfig& cfg) {
	ssimp::details::IosStateHolder _holder(os);
	os << std::boolalpha;
	os << std::format("-> {}\n\ttype: {}\n\ttext: {}\n", cfg.id(), cfg.type(),
	                  cfg.text());

	if (cfg.type() == "header")
		return os;

	os << "\tdefault: ";
	std::visit([&](auto x) -> void { os << x << '\n'; }, cfg.default_());

	if (cfg.type() == "checkbox")
		return os;

	if (cfg.type() == "choice") {
		os << "\tpossible values: [ ";
		for (const auto& val : cfg.values())
			os << '\'' << val << "' ";
		os << "]\n";
		return os;
	}

	if (cfg.type() == "subsection") {
		for (const auto& option : cfg.options()) {
			std::ostringstream oss;
			oss << option;
			os << '\t' << ssimp::details::indent(oss.str()) << '\n';
		}
		return os;
	}

	os << '\t' << ((cfg.type() == "text") ? "size " : "") << "range: < ";
	std::visit([&](auto x, auto y) -> void { os << x << " : " << y << " >\n"; },
	           cfg.range()[0], cfg.range()[1]);

	return os;
}
