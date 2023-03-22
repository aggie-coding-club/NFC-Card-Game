/*
 *  This file contains metaprogramming constructs necessary to make MultitaskingFramework easier to use.
 *  Most of the definitions here are for doing things at compile-time that make the code leaner and more user-friendly.
 */

// Implementation of is_same<A, B>
template<class T, class U>
struct is_same {
    enum { value = 0 };
};
template<class T>
struct is_same<T, T> {
    enum { value = 1 };
};

// Allow checking conditional statements on parameter packs
template<bool...> struct bool_pack;
template<bool... bs> 
using all_true = is_same<bool_pack<bs..., true>, bool_pack<true, bs...>>;
