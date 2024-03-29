#pragma once
#include <boost/json.hpp>
#include <boost/regex.hpp>
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ssimp {
class ExtensionManager {
  public:
	/**
	 * Register **extension** of a given **format**.
	 * The **extension** is the suffix after the last dot. (the dot is not
	 * included).
	 *
	 * When **regex** is set to true, the **extension** is interpreted as
	 * regular expression. Matching a regex is done in *fullmatch* mode, that is
	 * the expression must match the whole suffix in order to be accepted as
	 * compatible.
	 */
	void register_extension(const std::string& format,
	                        const std::string& extension,
	                        bool regex = false);

	/**
	 * Remove registered **extension**. Does nothing if the extension is not
	 * registered in the format.
	 */
	void remove_extension(const std::string& format,
	                      const std::string& extension,
	                      bool regex = false);
	/**
	 * Remove whole registered format. Does nothing, when the format is not
	 * registered.
	 */
	void remove_format(const std::string& format);

	/**
	 * Return set of all registered formats. All formats has at least one
	 * registered extension. (raw or regex).
	 */
	std::unordered_set<std::string> registered_formats() const;

	/**
	 * Return a vector of raw suffixes regsitered to a given **format**.
	 */
	const std::vector<std::string>&
	format_raw_suffixes(const std::string& format) const;

	/**
	 * Return a vector of regex suffixes registered to a given **format**.
	 */
	const std::vector<boost::regex>&
	format_regex_suffixes(const std::string& format) const;

	/**
	 * Return list of formats that are compatible with **file**.
	 *
	 * Even if format matches with more extensions (e.g. with raw as well as
	 * regex), it appears only once in the output.
	 *
	 * Formats matching with raw extension are always before formats matching
	 * with regex extension.
	 */
	std::vector<std::string>
	find_possible_formats(const std::filesystem::path& file) const;

	/**
	 * Return sorted formats by priority.
	 *
	 * To contrast with **find_possible_formats**, this function returns also
	 * the incompatible formats.
	 *
	 * Formats are sorted in the following order:
	 * compatible_raw, compatible_regex, incompatible_raw, incompatible_regex
	 *
	 * All formats appears only once in the output.
	 */
	std::vector<std::string>
	sorted_formats_by_priority(const std::filesystem::path& file) const;

	/**
	 * Set default extension to a **format**.
	 */
	void set_default_extension(const std::string& format,
	                           const std::string& extension);

	/**
	 * Get output extension.
	 */
	const std::string& get_default_extension(const std::string& format) const;

	/**
	 * Update filename **file** to have supported output extension of
	 * **format**.
	 */
	std::filesystem::path
	with_correct_extension(const std::string& format,
	                       const std::filesystem::path& file) const;

	/**
	 * Load format informations from json configuration.
	 */
	void load_from_json(const std::string& format,
	                    const boost::json::array& matchers,
	                    const boost::json::string& output_ext);

	/**
	 * Return true if extension of **file** is correct for **format**
	 */
	bool is_extension_correct(const std::string& format,
	                          const std::filesystem::path& file) const;

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
	std::unordered_map<std::string, std::string> _default_extensions;
};
} // namespace ssimp
