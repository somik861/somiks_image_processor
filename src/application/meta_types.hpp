#pragma once

#include <tuple>
#include <type_traits>

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
struct is_type_of_tuple : public std::false_type {};

template <typename T, typename... types_t>
struct is_type_of_tuple<T, std::tuple<types_t...>> {
	static constexpr bool value = is_any_of_v<T, types_t...>;
};

template <typename T, typename tuple_t>
constexpr bool is_type_of_tuple_v = is_type_of_tuple<T, tuple_t>::value;

template <typename T, typename... tuple_t>
struct tuple_type_idx {};

template <typename T, typename... types_t>
struct tuple_type_idx<T, std::tuple<types_t...>>
    : public type_idx<T, types_t...> {};

template <typename T, typename tuple_t>
constexpr std::size_t tuple_type_idx_v = tuple_type_idx<T, tuple_t>::value;

} // namespace traits

namespace concepts {
template <typename T, typename tuple_t>
concept TupleType = requires { traits::is_type_of_tuple_v<T, tuple_t>; };
}
} // namespace ssimp::mt
