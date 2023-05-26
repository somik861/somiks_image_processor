#pragma once
#include <boost/json.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>

namespace ssimp {
class OptionsManager {
  public:
	using value_t = std::variant<bool, int32_t, double, std::string>;
	using options_t = std::unordered_map<std::string, std::string>;
	void load_from_json(const std::string& format,
	                    const boost::json::value& json);

	options_t finalize_options(const std::string& format,
	                           const options_t& options) const;

	bool is_valid(const std::string& format, const options_t& options) const;

  private:
	std::optional<boost::json::object>
	_get_option(const boost::json::array& array,
	            const std::string& var_name) const;

	std::unordered_map<std::string, boost::json::array> _format_options;
};
} // namespace ssimp
