#pragma once
#include <string>
#include <unordered_set>

namespace ssimp {
class LicenseManager {
  public:
	std::unordered_set<std::string> available_licenses;
};
} // namespace ssimp
