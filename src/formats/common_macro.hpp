#pragma once
#include "common.hpp"

#define INSTANTIATE_SAVE_TEMPLATE(format, type)                                \
	template void format::save_image(                                          \
	    const std::vector<::ssimp::img::ndImage<type>>&, const fs::path&,      \
	    const ::ssimp::option_types::options_t&);
