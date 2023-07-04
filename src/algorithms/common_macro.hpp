#pragma once

#include "common.hpp"

#define INSTANTIATE_TEMPLATE(algorithm, type)                                  \
	template std::vector<img::LocalizedImage> algorithm::apply(                \
	    const std::vector<::ssimp::img::ndImage<type>>&,                       \
	    const ::ssimp::option_types::options_t&);
