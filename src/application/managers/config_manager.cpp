#include "config_manager.hpp"
#include <istream>

#ifdef INLINE_CONFIGS
#include "inlined_configs.hpp"
#endif

namespace fs = std::filesystem;

namespace ssimp {
boost::json::value ConfigManager::load_format(const std::string& format) const {
#ifdef INLINE_CONFIGS
	return boost::json::parse(ssimp::inlined_configs::formats.at(format));
#else
	return _load_json(_format_folder / (format + ".json"));
#endif
}
boost::json::value
ConfigManager::load_algorithm(const std::string& algorithm) const {
#ifdef INLINE_CONFIGS
	return boost::json::parse(ssimp::inlined_configs::algorithms.at(algorithm));
#else
	return _load_json(_algo_folder / (algorithm + ".json"));
#endif
}

boost::json::value ConfigManager::_load_json(const fs::path& file) const {
	std::ifstream f(file);
	return boost::json::parse(f);
}

} // namespace ssimp
