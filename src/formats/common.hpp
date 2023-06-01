#pragma once

#include "../application/managers/options_manager.hpp"
#include "../application/nd_image.hpp"
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <vector>

namespace fs = std::filesystem;

#define INSTANTIATE_SAVE_TEMPLATE(format, type)                                \
	template void format::save_image(const ::ssimp::img::ndImage<type>&,       \
	                                 const fs::path&,                          \
	                                 const ::ssimp::option_types::options_t&);

namespace ssimp::formats::details {
inline std::vector<std::byte> read_file(fs::path& path) {
	std::ifstream file(path, std::ios::binary);
	std::vector<std::byte> out;

	while (file.good())
		out.push_back(std::byte(file.get()));

	out;
}

} // namespace ssimp::formats::details
