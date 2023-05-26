#include "options_manager.hpp"

#include <cassert>
#include <string_view>

using namespace std::literals;

namespace ssimp {
void OptionsManager::load_from_json(const std::string& format,
                                    const boost::json::value& json) {
	assert(!_format_options.contains(format));
	assert(json.is_array());

	_format_options[format] = json.as_array();
}

// TODO
OptionsManager::options_t OptionsManager::finalize_options(
    const std::string& format, const OptionsManager::options_t& options) const {
	return options;
}

// TODO
bool OptionsManager::is_valid(const std::string& format,
                              const options_t& options) const {
	return false;
}

std::optional<boost::json::object>
OptionsManager::_get_option(const boost::json::array& array,
                            const std::string& var_name) const {
	for (boost::json::value value : array) {
		assert(value.is_object());

		boost::json::object obj = value.as_object();
		assert(obj.contains("type"));
		assert(obj.at("type").is_string());
		std::string_view type = obj.at("type").as_string();

		if (type == "header"sv)
			continue;

		assert(obj.contains("var_name"));
		assert(obj.at("var_name").is_string());

		if (obj.at("var_name").as_string() == var_name)
			return obj;

		if (type == "subsection") {
			assert(obj.contains("options"));
			assert(obj.at("options").is_array());

			auto option = _get_option(obj.at("options").as_array(), var_name);
			if (option)
				return option;
		}
	}

	return {};
}

} // namespace ssimp
