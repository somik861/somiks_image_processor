#include "options_manager.hpp"

#include <cassert>
#include <string_view>
#include <unordered_set>

#define CHECK(x)                                                               \
	if (!(x))                                                                  \
		return false;

using namespace std::literals;

namespace {
std::vector<std::string> string_array_to_vector(const boost::json::array& arr) {
	std::vector<std::string> out;

	for (auto value : arr)
		out.emplace_back(value.get_string());
	return out;
}

void finalize_options_rec(
    const std::vector<ssimp::option_types::OptionConfig>& option_cfgs,
    ssimp::option_types::options_t& options) {

	for (const auto& cfg : option_cfgs) {
		if (cfg.type() == "header")
			continue;

		if (!options.contains(cfg.id())) {
			if (cfg.type() == "int")
				options[cfg.id()] = cfg.default_as<int32_t>();

			if (cfg.type() == "double")
				options[cfg.id()] = cfg.default_as<double>();

			if (cfg.type() == "text" || cfg.type() == "choice")
				options[cfg.id()] = cfg.default_as<std::string>();

			if (cfg.type() == "checkbox" || cfg.type() == "subsection")
				options[cfg.id()] = cfg.default_as<bool>();
		}
		if (cfg.type() == "subsection" && std::get<bool>(options[cfg.id()]))
			finalize_options_rec(cfg.options(), options);
	}
}

void get_option_and_deps_rec(
    std::optional<ssimp::option_types::OptionConfig>& option,
    std::unordered_set<std::string>& deps,
    const std::vector<ssimp::option_types::OptionConfig> configs,
    const std::string& id) {
	for (const ssimp::option_types::OptionConfig& config : configs) {
		if (config.type() == "header")
			continue;

		if (config.id() == id) {
			option = config;
			return;
		}

		if (config.type() == "subsection") {
			deps.insert(config.id());
			get_option_and_deps_rec(option, deps, config.options(), id);
			if (option)
				return;
			deps.erase(config.id());
		}
	}
}

std::vector<ssimp::option_types::OptionConfig>
load_from_json_rec(const boost::json::array& json) {

	std::vector<ssimp::option_types::OptionConfig> configs;

	for (const auto& option_ : json) {
		boost::json::object option = option_.get_object();
		ssimp::option_types::OptionConfig option_config;

		std::string opt_type(option.at("type").get_string());
		option_config.type() = opt_type;
		option_config.text() = option.at("text").get_string();

		if (opt_type != "header")
			option_config.id() = option.at("id").get_string();

		auto opt_default = option.at("default");

		if (opt_type == "choice") {
			option_config.default_() = std::string(opt_default.get_string());
			option_config.values() =
			    string_array_to_vector(option.at("values").as_array());
		}

		if (opt_type == "checkbox")
			option_config.default_() = opt_default.get_bool();

		if (opt_type == "subsection") {
			option_config.default_() = opt_default.get_bool();
			option_config.options() =
			    load_from_json_rec(option.at("options").get_array());
		}

		if (opt_type == "int" || opt_type == "double" || opt_type == "text") {
			auto opt_range_0 = option.at("range").get_array()[0];
			auto opt_range_1 = option.at("range").get_array()[1];

			if (opt_type == "int") {
				option_config.default_() = int32_t(opt_default.get_int64());
				option_config.range()[0] = int32_t(opt_range_0.get_int64());
				option_config.range()[1] = int32_t(opt_range_1.get_int64());
			}

			if (opt_type == "double") {
				option_config.default_() = opt_default.get_double();
				option_config.range() = {opt_range_0.get_double(),
				                         opt_range_1.get_double()};
			}

			if (opt_type == "text") {
				option_config.default_() =
				    std::string(opt_default.get_string());
				option_config.range()[0] = int32_t(opt_range_0.get_int64());
				option_config.range()[1] = int32_t(opt_range_1.get_int64());
			}
		}
		configs.push_back(option_config);
	}

	return configs;
}

} // namespace

namespace ssimp {
void OptionsManager::load_from_json(const std::string& identifier,
                                    const boost::json::array& json) {
	assert(!_loaded_configs.contains(identifier));
	_loaded_configs[identifier] = load_from_json_rec(json);
}

option_types::options_t
OptionsManager::finalize_options(const std::string& identifier,
                                 const option_types::options_t& options) const {
	option_types::options_t finalized = options;

	finalize_options_rec(_loaded_configs.at(identifier), finalized);

	return finalized;
}

bool OptionsManager::is_valid(const std::string& identifier,
                              const option_types::options_t& options) const {

	for (const auto& [name, value] : options) {
		const auto& [option, deps] = _get_option_and_deps(identifier, name);
		CHECK(option);

		for (const auto& dep : deps) {
			CHECK((options.contains(dep) &&
			       std::holds_alternative<bool>(options.at(dep)) &&
			       std::get<bool>(options.at(dep))) ||
			      _get_option_and_deps(identifier, dep)
			          .first->default_as<bool>());
		}

		if (option->type() == "int") {
			CHECK(std::holds_alternative<int32_t>(value));

			int32_t raw_value = std::get<int32_t>(value);
			auto value_range = option->range_as<int32_t>();

			CHECK(value_range[0] <= raw_value);
			CHECK(raw_value <= value_range[1]);
		} else if (option->type() == "double") {
			CHECK(std::holds_alternative<double>(value));

			double raw_value = std::get<double>(value);
			auto value_range = option->range_as<double>();

			CHECK(value_range[0] <= raw_value);
			CHECK(raw_value <= value_range[1]);
		} else if (option->type() == "text") {
			CHECK(std::holds_alternative<std::string>(value));

			int32_t value_size = int32_t(std::get<std::string>(value).size());
			auto value_range = option->range_as<int32_t>();

			CHECK(value_range[0] <= value_size);
			CHECK(value_size <= value_range[1]);
		} else if (option->type() == "choice") {
			CHECK(std::holds_alternative<std::string>(value));

			const std::string& raw_value = std::get<std::string>(value);
			CHECK(std::ranges::count(option->values(), raw_value));
		} else if (option->type() == "checkbox") {
			CHECK(std::holds_alternative<bool>(value));
		} else if (option->type() == "subsection") {
			CHECK(std::holds_alternative<bool>(value));
		} else
			return false;
	}
	return true;
}

const std::vector<option_types::OptionConfig>&
OptionsManager::option_configs(const std::string& identifier) const {
	return _loaded_configs.at(identifier);
}

std::pair<std::optional<option_types::OptionConfig>,
          std::unordered_set<std::string>>
OptionsManager::_get_option_and_deps(const std::string& identifier,
                                     const std::string& option_id) const {
	std::optional<option_types::OptionConfig> option;
	std::unordered_set<std::string> deps;

	get_option_and_deps_rec(option, deps, _loaded_configs.at(identifier),
	                        option_id);

	return {option, deps};
}

} // namespace ssimp
