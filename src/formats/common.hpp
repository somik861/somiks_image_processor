#pragma once

#include "../application/managers/options_manager.hpp"
#include "../application/nd_image.hpp"
#include "../application/utils.hpp"
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace fs = std::filesystem;

#define INSTANTIATE_SAVE_TEMPLATE(format, type)                                \
	template void format::save_image(const ::ssimp::img::ndImage<type>&,       \
	                                 const fs::path&,                          \
	                                 const ::ssimp::option_types::options_t&);

namespace ssimp::formats::details {
inline std::vector<std::byte> read_file(const fs::path& path) {
	std::ifstream file(path, std::ios::binary);
	std::vector<std::byte> out;

	while (file.good())
		out.push_back(std::byte(file.get()));

	return out;
}

inline void save_file(const fs::path& path, std::span<const std::byte> bytes) {
	fs::create_directories(path.parent_path());

	std::ofstream file(path, std::ios::binary);

	for (auto byte : bytes)
		file.put(static_cast<unsigned char>(byte));
}
} // namespace ssimp::formats::details
