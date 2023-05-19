#pragma once
#include <boost/regex.hpp>
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class ExtensionMatcher {
  public:
	void register_extension(const std::string& format,
	                        const std::string& extension,
	                        bool regex = false);

	void remove_extension(const std::string& format,
	                      const std::string& extension,
	                      bool regex = false);
	void remove_format(const std::string& format);

	std::unordered_set<std::string> registered_formats() const;
	const std::vector<std::string>&
	format_raw_suffixes(const std::string& format) const;
	const std::vector<boost::regex>&
	format_regex_suffixes(const std::string& format) const;

	std::vector<std::string>
	find_possible_formats(const std::filesystem::path& file) const;

	std::vector<std::string>
	sorted_formats_by_priority(const std::filesystem::path& file) const;

  private:
	std::string _get_extension(const std::filesystem::path& file) const;
	std::pair<std::vector<std::string>, std::vector<std::string>>
	_divide_matching_nonmatching_raw(const std::string& ext) const;

	std::pair<std::vector<std::string>, std::vector<std::string>>
	_divide_matching_nonmatching_regex(const std::string& ext) const;

	std::unordered_map<std::string, std::vector<std::string>>
	    _format_raw_suffixes;
	std::unordered_map<std::string, std::vector<boost::regex>>
	    _format_regex_suffixes;
};
