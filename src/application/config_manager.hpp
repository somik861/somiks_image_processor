#pragma once

#include <boost/dll.hpp>
#include <boost/json.hpp>
#include <filesystem>
#include <ostream>

namespace ssimp {
class ConfigManager {
  public:
	ConfigManager() = default;
	boost::json::value load_format(const std::string& format) const;
	boost::json::value load_algorithm(const std::string& algorithm) const;

	friend std::ostream& operator<<(std::ostream& os,
	                                const ConfigManager& cfg) {
		os << "ConfigManager properties:\n";
		os << "Binary dir: " << cfg._binary_dir << '\n';
		os << "Format folder: " << cfg._format_folder << '\n';
		os << "Algo folder: " << cfg._algo_folder << '\n';
		return os;
	}

  private:
	boost::json::value _load_json(const std::filesystem::path& file) const;

	const std::filesystem::path _binary_dir =
	    boost::dll::program_location().parent_path().c_str();
	const std::filesystem::path _format_folder = _binary_dir / "formats";
	const std::filesystem::path _algo_folder = _binary_dir / "algorithms";
};
} // namespace ssimp
