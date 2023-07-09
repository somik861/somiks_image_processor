#pragma once

#include <tuple>
#include <type_traits>
#include <variant>

namespace ssimp::mt {
namespace traits {
template <typename T, typename... types_t>
using is_any_of = std::disjunction<std::is_same<T, types_t>...>;

template <typename T, typename... types_t>
constexpr bool is_any_of_v = is_any_of<T, types_t...>::value;

template <typename T, std::size_t I, typename... types>
struct _type_idx {};

template <typename T, std::size_t I, typename first_t, typename... types_t>
struct _type_idx<T, I, first_t, types_t...>
    : public std::conditional_t<std::is_same_v<first_t, T>,
                                std::integral_constant<std::size_t, I>,
                                _type_idx<T, I + 1, types_t...>> {};

template <typename T, typename... types_t>
struct type_idx : public _type_idx<T, 0, types_t...> {
	static_assert(is_any_of_v<T, types_t...>,
	              "Searched type does not belong to types to search in");
};

template <typename T, typename... types_t>
constexpr int type_idx_v = type_idx<T, types_t...>::value;

template <typename T, typename tuple_t>
struct is_any_of_tuple : public std::false_type {};

template <typename T, typename... types_t>
struct is_any_of_tuple<T, std::tuple<types_t...>> {
	static constexpr bool value = is_any_of_v<T, types_t...>;
};

template <typename T, typename tuple_t>
constexpr bool is_any_of_tuple_v = is_any_of_tuple<T, tuple_t>::value;

template <typename T, typename... tuple_t>
struct tuple_type_idx {};

template <typename T, typename... types_t>
struct tuple_type_idx<T, std::tuple<types_t...>>
    : public type_idx<T, types_t...> {};

template <typename T, typename tuple_t>
constexpr std::size_t tuple_type_idx_v = tuple_type_idx<T, tuple_t>::value;

template <typename needle_t, typename haystack_t>
struct is_subset_of;

template <typename... types_t, typename haystack_t>
struct is_subset_of<std::tuple<types_t...>, haystack_t>
    : public std::conjunction<is_any_of_tuple<types_t, haystack_t>...> {};

template <typename needle_t, typename haystack_t>
constexpr bool is_subset_of_v = is_subset_of<needle_t, haystack_t>::value;

template <typename variant_t, typename T>
struct variant_append;

template <typename... types_t, typename T>
struct variant_append<std::variant<types_t...>, T> {
	using type = std::variant<types_t..., T>;
};

template <typename variant_t, typename T>
using variant_append_t = variant_append<variant_t, T>::type;

template <typename T, typename variant_t>
struct variant_prepend;

template <typename T, typename... types_t>
struct variant_prepend<T, std::variant<types_t...>> {
	using type = std::variant<T, types_t...>;
};

template <typename T, typename variant_t>
using variant_prepend_t = variant_prepend<T, variant_t>::type;

template <typename variant_t>
struct variant_to_tuple;

template <typename... types_t>
struct variant_to_tuple<std::variant<types_t...>> {
	using type = std::tuple<types_t...>;
};

template <typename variant_t>
using variant_to_tuple_t = variant_to_tuple<variant_t>::type;

template <typename tuple_t>
struct tuple_to_variant;

template <typename... types_t>
struct tuple_to_variant<std::tuple<types_t...>> {
	using type = std::variant<types_t...>;
};

template <typename tuple_t>
using tuple_to_variant_t = tuple_to_variant<tuple_t>::type;
} // namespace traits

namespace concepts {
template <typename T, typename tuple_t>
concept TupleType = requires { traits::is_any_of_tuple_v<T, tuple_t>; };
}
} // namespace ssimp::mt
