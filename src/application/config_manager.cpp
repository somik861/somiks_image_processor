#include "config_manager.hpp"
#include <istream>

namespace fs = std::filesystem;

namespace ssimp {
boost::json::value ConfigManager::load_format(const std::string& format) const {
	return _load_json(_format_folder / (format + ".json"));
}
boost::json::value
ConfigManager::load_algorithm(const std::string& algorithm) const {
	return _load_json(_algo_folder / (algorithm + ".json"));
}

boost::json::value ConfigManager::_load_json(const fs::path& file) const {
	std::ifstream f(file);
	return boost::json::parse(f);
}

} // namespace ssimp
