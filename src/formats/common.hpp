#pragma once

#include "../application/nd_image.hpp"

namespace {
template <typename format_t, typename types>
struct generate_templates_impl {
	static void do_not_call_me() { std::terminate(); }
};

template <typename format_t, typename this_t, typename... rest_t>
struct generate_templates_impl<format_t, std::tuple<this_t, rest_t...>> {
	static void do_not_call_me() {
		format_t::save_image(ssimp::img::ndImage<this_t>(1), {}, {});

		generate_templates_impl<format_t,
		                        std::tuple<rest_t...>>::do_not_call_me();
	}
};

template <typename format_t>
struct generate_templates {
	static void do_not_call_me() {
		generate_templates_impl<
		    format_t, typename format_t::supported_types>::do_not_call_me();
	}
};
} // namespace
