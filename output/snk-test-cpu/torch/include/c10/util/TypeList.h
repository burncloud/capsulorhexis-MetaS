#pragma once

#include <c10/util/C++17.h>
#include <c10/util/TypeTraits.h>

namespace c10 { namespace guts {

template<class... T> struct false_t : std::false_type {};
template<template<class> class... T> struct false_higher_t : std::false_type {};

namespace typelist {



/**
 * Type holding a list of types for compile time type computations
 */
template<class... Items> struct typelist final {
private:
    typelist() = delete; // not for instantiation
};



/**
 * Returns the number of types in a typelist
 * Example:
 *   3  ==  size<typelist<int, int, double>>::value
 */
template<class TypeList> struct size final {
    static_assert(false_t<TypeList>::value, "In typelist::size<T>, T must be typelist<...>.");
};
template<class... Types> struct size<typelist<Types...>> final {
    static constexpr size_t value = sizeof...(Types);
};



/**
 * Transforms a list of types into a tuple holding these types.
 * Example:
 *   std::tuple<int, string>  ==  to_tuple_t<typelist<int, string>>
 */
template<class TypeList> struct to_tuple final {
    static_assert(false_t<TypeList>::value, "In typelist::to_tuple<T>, T must be typelist<...>.");
};
template<class... Types> struct to_tuple<typelist<Types...>> final {
    using type = std::tuple<Types...>;
};
template<class TypeList> using to_tuple_t = typename to_tuple<TypeList>::type;




/**
 * Creates a typelist containing the types of a given tuple.
 * Example:
 *   typelist<int, string>  ==  from_tuple_t<std::tuple<int, string>>
 */
template<class Tuple> struct from_tuple final {
    static_assert(false_t<Tuple>::value, "In typelist::from_tuple<T>, T must be std::tuple<...>.");
};
template<class... Types> struct from_tuple<std::tuple<Types...>> final {
  using type = typelist<Types...>;
};
template<class Tuple> using from_tuple_t = typename from_tuple<Tuple>::type;



/**
 * Concatenates multiple type lists.
 * Example:
 *   typelist<int, string, int>  ==  concat_t<typelist<int, string>, typelist<int>>
 */
template<class... TypeLists> struct concat final {
    static_assert(false_t<TypeLists...>::value, "In typelist::concat<T1, ...>, the T arguments each must be typelist<...>.");
};
template<class... Head1Types, class... Head2Types, class... TailLists>
struct concat<typelist<Head1Types...>, typelist<Head2Types...>, TailLists...> final {
  using type = typename concat<typelist<Head1Types..., Head2Types...>, TailLists...>::type;
};
template<class... HeadTypes>
struct concat<typelist<HeadTypes...>> final {
  using type = typelist<HeadTypes...>;
};
template<>
struct concat<> final {
  using type = typelist<>;
};
template<class... TypeLists> using concat_t = typename concat<TypeLists...>::type;



/**
 * Filters the types in a type list by a type trait.
 * Examples:
 *   typelist<int&, const string&&>  ==  filter_t<std::is_reference, typelist<void, string, int&, bool, const string&&, int>>
 */
template<template <class> class Condition, class TypeList> struct filter final {
  static_assert(false_t<TypeList>::value, "In typelist::filter<Condition, TypeList>, the TypeList argument must be typelist<...>.");
};
template<template <class> class Condition, class Head, class... Tail>
struct filter<Condition, typelist<Head, Tail...>> final {
  static_assert(is_type_condition<Condition>::value, "In typelist::filter<Condition, TypeList>, the Condition argument must be a condition type trait, i.e. have a static constexpr bool ::value member.");
  using type = guts::conditional_t<
    Condition<Head>::value,
    concat_t<typelist<Head>, typename filter<Condition, typelist<Tail...>>::type>,
    typename filter<Condition, typelist<Tail...>>::type
  >;
};
template<template <class> class Condition>
struct filter<Condition, typelist<>> final {
  static_assert(is_type_condition<Condition>::value, "In typelist::filter<Condition, TypeList>, the Condition argument must be a condition type trait, i.e. have a static constexpr bool ::value member.");
  using type = typelist<>;
};
template<template <class> class Condition, class TypeList>
using filter_t = typename filter<Condition, TypeList>::type;



/**
 * Counts how many types in the list fulfill a type trait
 * Examples:
 *   2  ==  count_if<std::is_reference, typelist<void, string, int&, bool, const string&&, int>>
 */
template<template <class> class Condition, class TypeList>
struct count_if final {
  static_assert(is_type_condition<Condition>::value, "In typelist::count_if<Condition, TypeList>, the Condition argument must be a condition type trait, i.e. have a static constexpr bool ::value member.");
  static_assert(is_instantiation_of<typelist, TypeList>::value, "In typelist::count_if<Condition, TypeList>, the TypeList argument must be typelist<...>.");
  // TODO Direct implementation might be faster
  static constexpr size_t value = size<filter_t<Condition, TypeList>>::value;
};


/**
 * Checks if a typelist contains a certain type.
 * Examples:
 *  contains<typelist<int, string>, string> == true_type
 *  contains<typelist<int, string>, double> == false_type
 */
namespace detail {
template<class TypeList, class Type, class Enable = void> struct contains {};
template<class Type> struct contains<typelist<>, Type, void> : std::false_type {};
template<class Type, class Head, class... Tail>
struct contains<typelist<Head, Tail...>, Type, guts::enable_if_t<std::is_same<Head, Type>::value>> : std::true_type {};
template<class Type, class Head, class... Tail>
struct contains<typelist<Head, Tail...>, Type, guts::enable_if_t<!std::is_same<Head, Type>::value>> : contains<typelist<Tail...>, Type> {};
}
template<class TypeList, class Type>
using contains = typename detail::contains<TypeList, Type>::type;


/**
 * Returns true iff the type trait is true for all types in the type list
 * Examples:
 *   true   ==  true_for_each_type<std::is_reference, typelist<int&, const float&&, const MyClass&>>::value
 *   false  ==  true_for_each_type<std::is_reference, typelist<int&, const float&&, MyClass>>::value
 */
template<template <class> class Condition, class TypeList> struct true_for_each_type final {
    static_assert(false_t<TypeList>::value, "In typelist::true_for_each_type<Condition, TypeList>, the TypeList argument must be typelist<...>.");
};
template<template <class> class Condition, class... Types>
struct true_for_each_type<Condition, typelist<Types...>> final
: guts::conjunction<Condition<Types>...> {
    static_assert(is_type_condition<Condition>::value, "In typelist::true_for_each_type<Condition, TypeList>, the Condition argument must be a condition type trait, i.e. have a static constexpr bool ::value member.");
};



/**
 * Maps types of a type list using a type trait
 * Example:
 *  typelist<int&, double&, string&>  ==  map_t<std::add_lvalue_reference_t, typelist<int, double, string>>
 */
template<template <class> class Mapper, class TypeList> struct map final {
    static_assert(false_t<TypeList>::value, "In typelist::map<Mapper, TypeList>, the TypeList argument must be typelist<...>.");
};
template<template <class> class Mapper, class... Types>
struct map<Mapper, typelist<Types...>> final {
  using type = typelist<Mapper<Types>...>;
};
template<template <class> class Mapper, class TypeList>
using map_t = typename map<Mapper, TypeList>::type;



/**
 * Returns the first element of a type list.
 * Example:
 *   int  ==  head_t<typelist<int, string>>
 */
template<class TypeList> struct head final {
    static_assert(false_t<TypeList>::value, "In typelist::head<T>, the T argument must be typelist<...>.");
};
template<class Head, class... Tail> struct head<typelist<Head, Tail...>> final {
  using type = Head;
};
template<class TypeList> using head_t = typename head<TypeList>::type;

/**
 * Returns the N-th element of a type list.
 * Example:
 * int == element_t<1, typelist<float, int, char>>
 */

/// Base template.
template<size_t Index, class TypeList> struct element final {
    static_assert(false_t<TypeList>::value, "In typelist::element<T>, the T argument must be typelist<...>.");
};

/// Successful case, we have reached the zero index and can "return" the head type.
template<class Head, class... Tail> struct element<0, typelist<Head, Tail...>> { using type = Head; };

/// Error case, we have an index but ran out of types! It will only be selected
/// if `Ts...` is actually empty!
template <size_t Index, class... Ts>
struct element<Index, typelist<Ts...>> {
  static_assert(Index < sizeof...(Ts), "Index is out of bounds in typelist::element");
};

/// Shave off types until we hit the <0, Head, Tail...> or <Index> case.
template<size_t Index, class Head, class... Tail> struct element<Index, typelist<Head, Tail...>> : element<Index-1, typelist<Tail...>> { };

/// Convenience alias.
template<size_t Index, class TypeList>
using element_t = typename element<Index, TypeList>::type;

/**
 * Returns the last element of a type list.
 * Example:
 *   int  ==  last_t<typelist<int, string>>
 */
template <class TypeList>
struct last final {
  static_assert(
      false_t<TypeList>::value,
      "In typelist::last<T>, the T argument must be typelist<...>.");
};
template <class Head, class... Tail>
struct last<typelist<Head, Tail...>> final {
  using type = typename last<typelist<Tail...>>::type;
};
template <class Head>
struct last<typelist<Head>> final {
  using type = Head;
};
template <class TypeList>
using last_t = typename last<TypeList>::type;
static_assert(
    std::is_same<int, last_t<typelist<double, float, int>>>::value,
    "");

/**
 * Reverses a typelist.
 * Example:
 *   typelist<int, string>  == reverse_t<typelist<string, int>>
 */
template<class TypeList> struct reverse final {
    static_assert(false_t<TypeList>::value, "In typelist::reverse<T>, the T argument must be typelist<...>.");
};
template<class Head, class... Tail> struct reverse<typelist<Head, Tail...>> final {
  using type = concat_t<typename reverse<typelist<Tail...>>::type, typelist<Head>>;
};
template<> struct reverse<typelist<>> final {
  using type = typelist<>;
};
template<class TypeList> using reverse_t = typename reverse<TypeList>::type;


/**
 * Find the index of the first type in a typelist fulfilling a type trait condition.
 * Example:
 *
 * 2 == find_if<typelist<char, int, char&, int&>, std::is_reference>::value
 */
template<class TypeList, template<class> class Condition, class Enable = void> struct find_if final {
  static_assert(false_t<TypeList>::value, "In typelist::find_if<TypeList, Condition>, the TypeList argument must be typelist<...>.");
};
template<template<class> class Condition> struct find_if<typelist<>, Condition, void> final {
  static_assert(false_higher_t<Condition>::value, "In typelist::find_if<Type/List, Condition>, didn't find any type fulfilling the Condition.");
};
template<class Head, class... Tail, template<class> class Condition>
struct find_if<typelist<Head, Tail...>, Condition, enable_if_t<Condition<Head>::value>> final {
  static constexpr size_t value = 0;
};
template<class Head, class... Tail, template<class> class Condition>
struct find_if<typelist<Head, Tail...>, Condition, enable_if_t<!Condition<Head>::value>> final {
  static constexpr size_t value = 1 + find_if<typelist<Tail...>, Condition>::value;
};



/**
 * Maps a list of types into a list of values.
 * Examples:
 *   // C++14 example
 *   auto sizes =
 *     map_types_to_values<typelist<int64_t, bool, uint32_t>>(
 *       [] (auto t) { return sizeof(decltype(t)::type); }
 *     );
 *   //  sizes  ==  std::tuple<size_t, size_t, size_t>{8, 1, 4}
 *
 *   // C++14 example
 *   auto shared_ptrs =
 *     map_types_to_values<typelist<int, double>>(
 *       [] (auto t) { return make_shared<typename decltype(t)::type>(); }
 *     );
 *   // shared_ptrs == std::tuple<shared_ptr<int>, shared_ptr<double>>()
 *
 *   // C++11 example
 *   struct map_to_size {
 *     template<class T> constexpr size_t operator()(T) {
 *       return sizeof(typename T::type);
 *     }
 *   };
 *   auto sizes =
 *     map_types_to_values<typelist<int64_t, bool, uint32_t>>(
 *       map_to_size()
 *     );
 *   //  sizes  ==  std::tuple<size_t, size_t, size_t>{8, 1, 4}
 */
namespace detail {
template<class T> struct type_ final {
    using type = T;
};
template<class TypeList> struct map_types_to_values final {
    static_assert(false_t<TypeList>::value, "In typelist::map_types_to_values<T>, the T argument must be typelist<...>.");
};
template<class... Types> struct map_types_to_values<typelist<Types...>> final {
  template<class Func>
  static std::tuple<guts::result_of_t<Func(type_<Types>)>...> call(Func&& func) {
    return std::tuple<guts::result_of_t<Func(type_<Types>)>...> { std::forward<Func>(func)(type_<Types>())... };
  }
};
}

template<class TypeList, class Func> auto map_types_to_values(Func&& func)
-> decltype(detail::map_types_to_values<TypeList>::call(std::forward<Func>(func))) {
  return detail::map_types_to_values<TypeList>::call(std::forward<Func>(func));
}


}}}
