#pragma once
#include <boost/dll.hpp>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace ssimp {
class LicenseManager {
  public:
	/**
	 * Custom c'tor for license loading.
	 */
	LicenseManager();
	/**
	 * Get set of avaiable licenses
	 */
	const std::unordered_set<std::string>& available_licenses() const;
	/**
	 * Obtain license text
	 */
	const std::string& license(const std::string& name) const;

  private:
	std::unordered_set<std::string> _licence_names;
	/**
	 * Cache of loaded licenses in RAM.
	 */
	mutable std::unordered_map<std::string, std::string> _loaded;
	std::filesystem::path _license_folder =
	    (boost::dll::program_location().parent_path() / "licenses").c_str();
};
} // namespace ssimp
