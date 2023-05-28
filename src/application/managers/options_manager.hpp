#pragma once
#include <boost/json.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>

namespace ssimp {
class OptionsManager {
  public:
	/**
	 * Variant of possible option values
	 */
	using value_t = std::variant<bool, int32_t, double, std::string>;
	/**
	 * Type representing a set of options
	 */
	using options_t = std::unordered_map<std::string, value_t>;

	/**
	 * Load options configuration of **identifier** from json array
	 */
	void load_from_json(const std::string& identifier,
	                    const boost::json::array& json);

	/**
	 * Return completed options, that is with added all default values.
	 * Values located in disabled subsections are not added.
	 *
	 * **is_valid** on **options** must evaluate to **true**.
	 */
	options_t finalize_options(const std::string& identifier,
	                           const options_t& options) const;

	/**
	 * Checks if **options** formed correctly.
	 *
	 * That is, correct var_names, formats, enabled necessary subsections, etc.
	 */
	bool is_valid(const std::string& identifier,
	              const options_t& options) const;

  private:
	std::pair<std::optional<boost::json::object>,
	          std::unordered_set<std::string>>
	_get_option_and_deps(const std::string& identifier,
	                     const std::string& var_name) const;

	std::unordered_map<std::string, boost::json::array> _loaded_configs;
};
} // namespace ssimp
