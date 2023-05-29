#include "extension_manager.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
namespace fs = std::filesystem;

namespace ssimp {
void ExtensionManager::register_extension(const std::string& format,
                                          const std::string& extension,
                                          bool regex /* = false */) {
	if (regex) {
		auto& extensions = _format_regex_suffixes[format];
		if (std::ranges::count(extensions, boost::regex(extension)) == 0)
			extensions.push_back(boost::regex(extension));
	} else {
		auto& extensions = _format_raw_suffixes[format];
		if (std::ranges::count(extensions, extension) == 0)
			extensions.push_back(extension);
	}
}

void ExtensionManager::remove_extension(const std::string& format,
                                        const std::string& extension,
                                        bool regex /* = false */) {
	if (regex) {
		auto& extensions = _format_regex_suffixes[format];
		if (std::ranges::count(extensions, boost::regex(extension)) != 0) {
			std::erase(extensions, boost::regex(extension));
			if (extensions.empty())
				_format_regex_suffixes.erase(format);
		}
	} else {
		auto& extensions = _format_raw_suffixes[format];
		if (std::ranges::count(extensions, extension) != 0) {
			std::erase(extensions, extension);
			if (extensions.empty())
				_format_raw_suffixes.erase(format);
		}
	}
}

void ExtensionManager::remove_format(const std::string& format) {
	_format_raw_suffixes.erase(format);
	_format_regex_suffixes.erase(format);
}

std::unordered_set<std::string> ExtensionManager::registered_formats() const {
	std::unordered_set<std::string> out;
	for (const auto& [k, _] : _format_raw_suffixes)
		out.insert(k);

	for (const auto& [k, _] : _format_regex_suffixes)
		out.insert(k);

	return out;
}

const std::vector<std::string>&
ExtensionManager::format_raw_suffixes(const std::string& format) const {
	return _format_raw_suffixes.at(format);
}

const std::vector<boost::regex>&
ExtensionManager::format_regex_suffixes(const std::string& format) const {
	return _format_regex_suffixes.at(format);
}

std::vector<std::string> ExtensionManager::find_possible_formats(
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

std::vector<std::string> ExtensionManager::sorted_formats_by_priority(
    const std::filesystem::path& file) const {
	std::string ext = _get_extension(file);

	std::vector<std::string> out;
	std::unordered_set<std::string> found;

	auto raw_division = _divide_matching_nonmatching_raw(ext);
	auto regex_division = _divide_matching_nonmatching_regex(ext);

	out.insert(out.end(), raw_division.first.begin(), raw_division.first.end());
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

std::string ExtensionManager::_get_extension(const fs::path& file) const {
	std::string out;

	auto ext = file.extension();
	for (auto ch : std::basic_string_view(ext.c_str())) {
		if (ch > 127)
			throw std::invalid_argument(
			    "File extension characters need to be ASCII symbols");

		if (ch == '.' and out.empty())
			continue;

		out.push_back(char(ch));
	}

	return out;
}

std::pair<std::vector<std::string>, std::vector<std::string>>
ExtensionManager::_divide_matching_nonmatching_raw(
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
ExtensionManager::_divide_matching_nonmatching_regex(
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

void ExtensionManager::set_output_extension(const std::string& format,
                                            const std::string& extension) {
	_output_extensions[format] = extension;
}

const std::string&
ExtensionManager::get_output_extension(const std::string& format) const {
	return _output_extensions.at(format);
}

fs::path ExtensionManager::with_output_extension(const std::string& format,
                                                 const fs::path& file) const {
	auto out = file;
	out.replace_extension(_output_extensions.at(format));

	return out;
}

void ExtensionManager::load_from_json(const std::string& format,
                                      const boost::json::array& matchers,
                                      const boost::json::string& output_ext) {
	for (auto match : matchers) {
		auto obj = match.get_object();
		bool regex =
		    obj.contains("type") && obj.at("type").get_string() == "regex";
		register_extension(format, std::string(obj.at("suffix").get_string()),
		                   regex);
	}

	set_output_extension(format, std::string(output_ext));
}

} // namespace ssimp
