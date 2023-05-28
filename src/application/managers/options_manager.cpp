#include "options_manager.hpp"

#include <cassert>
#include <string_view>
#include <unordered_set>

#define CHECK(x)                                                               \
	if (!(x))                                                                  \
		return false;

namespace {
std::unordered_set<std::string>
string_array_to_set(const boost::json::array& arr) {
	std::unordered_set<std::string> out;

	for (auto value : arr)
		out.emplace(value.get_string());
	return out;
}

void finalize_options_rec(const boost::json::array& option_cfgs,
                          ssimp::OptionsManager::options_t& options) {

	for (const auto& cfg : option_cfgs) {
		const auto& obj = cfg.get_object();
		std::string_view type = obj.at("type").get_string();

		if (type == "header")
			continue;

		std::string var_name = std::string(obj.at("var_name").get_string());
		if (!options.contains(var_name)) {
			if (type == "int")
				options[var_name] = int32_t(obj.at("default").get_int64());

			if (type == "double")
				options[var_name] = obj.at("default").get_double();

			if (type == "text" || type == "choice")
				options[var_name] = std::string(obj.at("default").get_string());

			if (type == "checkbox")
				options[var_name] = obj.at("defualt").get_bool();

			if (type == "subsection")
				options[var_name] = obj.at("default").get_bool();
		}
		if (type == "subsection" && std::get<bool>(options[var_name]))
			finalize_options_rec(obj.at("options").get_array(), options);
	}
}

void get_option_and_deps_rec(std::optional<boost::json::object>& option,
                             std::unordered_set<std::string>& deps,
                             const boost::json::array& array,
                             const std::string& var_name) {
	for (boost::json::value value : array) {

		boost::json::object obj = value.get_object();
		std::string_view type = obj.at("type").get_string();

		if (type == "header")
			continue;

		auto obj_var_name = std::string(obj.at("var_name").get_string());
		if (obj_var_name == var_name) {
			option = obj;
			return;
		}

		if (type == "subsection") {
			deps.insert(obj_var_name);
			get_option_and_deps_rec(option, deps, obj.at("options").get_array(),
			                        var_name);
			if (option)
				return;
			deps.erase(obj_var_name);
		}
	}
}

} // namespace

using namespace std::literals;

namespace ssimp {
void OptionsManager::load_from_json(const std::string& identifier,
                                    const boost::json::value& json) {
	assert(!_loaded_configs.contains(identifier));
	assert(json.is_array());

	_loaded_configs[identifier] = json.as_array();
}

OptionsManager::options_t OptionsManager::finalize_options(
    const std::string& identifier,
    const OptionsManager::options_t& options) const {
	OptionsManager::options_t finalized = options;

	finalize_options_rec(_loaded_configs.at(identifier), finalized);

	return finalized;
}

bool OptionsManager::is_valid(const std::string& identifier,
                              const options_t& options) const {
	for (const auto& [name, value] : options) {
		const auto& [option, deps] = _get_option_and_deps(identifier, name);
		CHECK(option);

		for (const auto& dep : deps) {
			CHECK((options.contains(dep) &&
			       std::holds_alternative<bool>(options.at(dep)) &&
			       std::get<bool>(options.at(dep))) ||
			      _get_option_and_deps(identifier, dep)
			          .first->at("default")
			          .get_bool());
		}

		std::string_view type = option->at("type").get_string();
		if (type == "int") {
			CHECK(std::holds_alternative<int32_t>(value));

			int32_t raw_value = std::get<int32_t>(value);
			auto value_range = option->at("range").get_array();
			int32_t lower_bound = int32_t(value_range.at(0).get_int64());
			int32_t upper_bound = int32_t(value_range.at(1).get_int64());

			CHECK(lower_bound <= raw_value);
			CHECK(raw_value <= upper_bound);
		} else if (type == "double") {
			CHECK(std::holds_alternative<double>(value));

			double raw_value = std::get<double>(value);
			auto value_range = option->at("range").get_array();
			double lower_bound = value_range.at(0).get_double();
			double upper_bound = value_range.at(1).get_double();

			CHECK(lower_bound <= raw_value);
			CHECK(raw_value <= upper_bound);
		} else if (type == "text") {
			CHECK(std::holds_alternative<std::string>(value));

			std::size_t value_size = std::get<std::string>(value).size();
			auto value_range = option->at("range").get_array();
			auto lower_bound = value_range.at(0).get_uint64();
			auto upper_bound = value_range.at(1).get_uint64();

			CHECK(lower_bound <= value_size);
			CHECK(value_size <= upper_bound);
		} else if (type == "choice") {
			CHECK(std::holds_alternative<std::string>(value));

			const std::string& raw_value = std::get<std::string>(value);
			CHECK(string_array_to_set(option->at("values").get_array())
			          .contains(raw_value));
		} else if (type == "checkbox") {
			CHECK(std::holds_alternative<bool>(value));
		} else if (type == "subsection") {
			CHECK(std::holds_alternative<bool>(value));
		} else
			return false;
	}

	return true;
}

std::pair<std::optional<boost::json::object>, std::unordered_set<std::string>>
OptionsManager::_get_option_and_deps(const std::string& identifier,
                                     const std::string& var_name) const {

	std::optional<boost::json::object> option;
	std::unordered_set<std::string> deps;

	get_option_and_deps_rec(option, deps, _loaded_configs.at(identifier),
	                        var_name);

	return {option, deps};
}

} // namespace ssimp
