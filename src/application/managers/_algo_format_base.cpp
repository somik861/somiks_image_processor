#include "_algo_format_base.hpp"

namespace ssimp::details {
bool _AlgoFormatBase::is_type_supported(const std::string& element,
                                        img::elem_type type) const {
	return _supported_types.at(element).contains(type);
}
const std::unordered_set<img::elem_type>&
_AlgoFormatBase::supported_types(const std::string& element) const {
	return _supported_types.at(element);
}
bool _AlgoFormatBase::is_count_supported(const std::string& element,
                                         std::size_t count) const {
	return _count_verifiers.at(element)(count);
}
bool _AlgoFormatBase::is_dims_supported(
    const std::string& element, std::span<const std::size_t> dims) const {
	return _dims_verifiers.at(element)(dims);
}
bool _AlgoFormatBase::is_same_dims_required(const std::string& element) const {
	return _same_dims_required.at(element);
}
std::unordered_set<std::string> _AlgoFormatBase::registered() const {
	std::unordered_set<std::string> out;
	out.reserve(_same_dims_required.size());

	for (const auto& [k, _] : _same_dims_required)
		out.insert(k);

	return out;
}

} // namespace ssimp::details
