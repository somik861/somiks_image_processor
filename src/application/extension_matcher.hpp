#pragma once
#include <string>
#include <unordered_map>
#include <vector>

class ExtensionMatcher {
  public:
  private:
	std::unordered_map<std::string, std::vector<std::string>>
	    _formats_raw_suffix;
	std::unordered_map<std::string, std::vector<std::string>>
	    _formats_regex_suffix;
};
