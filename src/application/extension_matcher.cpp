#include "extension_matcher.hpp"
#include <stdexcept>
namespace fs = std::filesystem;

void ExtensionMatcher::register_extension(const std::string& format,
                                          const std::string& extension,
                                          bool regex /* = false */) {
	if (regex)
		_format_regex_suffixes[format].push_back(boost::regex(extension));
	else
		_format_raw_suffixes[format].push_back(extension);
}

void ExtensionMatcher::remove_extension(const std::string& format,
                                        const std::string& extension,
                                        bool regex /* = false */) {
	if (regex)
		std::erase(_format_regex_suffixes[format], boost::regex(extension));
	else
		std::erase(_format_raw_suffixes[format], extension);
}

std::unordered_set<std::string> ExtensionMatcher::registered_formats() const {
	std::unordered_set<std::string> out;
	for (const auto& [k, v] : _format_raw_suffixes)
		out.insert(k);
	return out;
}

const std::vector<std::string>&
ExtensionMatcher::format_raw_suffixes(const std::string& format) const {
	return _format_raw_suffixes.at(format);
}

const std::vector<boost::regex>&
ExtensionMatcher::format_regex_suffixes(const std::string& format) const {
	return _format_regex_suffixes.at(format);
}

std::vector<std::string> ExtensionMatcher::find_possible_formats(
    const std::filesystem::path& file) const {
	std::string ext = _get_extension(file);
	std::vector<std::string> out;
	std::unordered_set<std::string> found;

	std::vector<std::string> matching_vec =
	    _divide_matching_nonmatching_raw(ext).first;
	out.insert(out.end(), matching_vec.begin(), matching_vec.end());
	found.insert(out.begin(), out.end());

	for (const auto& s : _divide_matching_nonmatching_regex(ext).first) {
		if (found.contains(s))
			break;

		// no need to add to found as it is unique withing regex matchings
		out.push_back(s);
	}

	return out;
}

std::vector<std::string> ExtensionMatcher::sorted_formats_by_priority(
    const std::filesystem::path& file) const {
	std::string ext = _get_extension(file);

	std::vector<std::string> out;
	std::unordered_set<std::string> found;

	auto raw_division = _divide_matching_nonmatching_raw(ext);
	auto regex_division = _divide_matching_nonmatching_regex(ext);

	out.insert(out.begin(), raw_division.first.begin(),
	           raw_division.first.end());
	found.insert(raw_division.first.begin(), raw_division.first.end());

	for (const auto& s : regex_division.first) {
		if (found.contains(s))
			continue;
		found.insert(s);
		out.push_back(s);
	}

	for (const auto& s : raw_division.second) {
		if (found.contains(s))
			continue;
		found.insert(s);
		out.push_back(s);
	}

	for (const auto& s : regex_division.second) {
		if (found.contains(s))
			continue;
		found.insert(s);
		out.push_back(s);
	}

	return out;
}

std::string ExtensionMatcher::_get_extension(const fs::path& file) const {
	std::string out;

	for (auto ch : std::basic_string_view(file.extension().c_str())) {
		if (ch == '.' and out.empty())
			continue;

		if (ch > 127)
			throw std::invalid_argument(
			    "File extension characters need to be ASCII symbols");

		out.push_back(char(ch));
	}

	return out;
}

std::pair<std::vector<std::string>, std::vector<std::string>>
ExtensionMatcher::_divide_matching_nonmatching_raw(
    const std::string& ext) const {
	std::vector<std::string> matching, nonmatching;

	for (const auto& [format, suffixes] : _format_raw_suffixes) {
		bool format_matches = false;
		for (const auto& suff : suffixes) {
			if (suff == ext) {
				format_matches = true;
				break;
			}
		}

		if (format_matches)
			matching.push_back(format);
		else
			nonmatching.push_back(format);
	}

	return {matching, nonmatching};
}

std::pair<std::vector<std::string>, std::vector<std::string>>
ExtensionMatcher::_divide_matching_nonmatching_regex(
    const std::string& ext) const {
	std::vector<std::string> matching, nonmatching;

	for (const auto& [format, suffixes] : _format_regex_suffixes) {
		bool format_matches = false;
		for (const auto& suff : suffixes) {
			if (boost::regex_match(ext, suff)) {
				format_matches = true;
				break;
			}
		}

		if (format_matches)
			matching.push_back(format);
		else
			nonmatching.push_back(format);
	}

	return {matching, nonmatching};
}
