#pragma once

#include "../application/managers/options_manager.hpp"
#include "../application/nd_image.hpp"
#include <filesystem>

namespace fs = std::filesystem;

#define INSTANTIATE_SAVE_TEMPLATE(format, type)                                \
	template void format::save_image(const ::ssimp::img::ndImage<type>&,       \
	                                 const fs::path&,                          \
	                                 const ::ssimp::option_types::options_t&);
