#include "license_manager.hpp"

namespace fs = std::filesystem;
#include <fstream>
#include <sstream>

namespace ssimp {
LicenseManager::LicenseManager() {
	for (const auto& entry : fs::directory_iterator(_license_folder))
		if (entry.path().extension() == ".lic")
			_licence_names.insert(entry.path().stem().string());
}

const std::unordered_set<std::string>&
LicenseManager::available_licenses() const {
	return _licence_names;
}
const std::string& LicenseManager::license(const std::string& name) const {
	if (!_loaded.contains(name)) {
		std::ifstream f(_license_folder / (name + ".lic"));
		std::stringstream ss;
		ss << f.rdbuf();
		_loaded[name] = std::move(ss.str());
	}

	return _loaded.at(name);
}
bool LicenseManager::is_license_available(const std::string& name) const {
	return _licence_names.contains(name);
}
} // namespace ssimp
