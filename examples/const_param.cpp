#include <cstddef>
#include <cstdio>
#include <meta>
#include <array>
#include <span>
#include <algorithm>

#include <cvl/cvl.hpp>

/*
    A 'cvl::const_param' allows us to
    life a function parameter into
    something which we can use as a
    template parameter.

    There are some limitations, though:

    Firstly, a function which uses a
    'cvl::const_param' cannot have its
    interface (e.g. its signature)
    depend on any 'cvl::const_param'.

    In particular, that means that we
    cannot use a deduced return type
    in concert with a 'cvl::const_param'.

    Secondly, when we use a 'cvl::const_param',
    we are actually implicitly involving
    a lambda in our template argument list.

    That means that every invocation of
    a function which uses a 'cvl::const_param'
    will be a brand new template instantiation,
    even when the values given to each
    'cvl::const_param' are identical to each other.
*/

/*
    In order to use a 'cvl::const_param',
    we first default an 'auto' template
    parameter to the expression 'cvl::const_param<T>'.

    Then, when we place it in the function's
    parameter list, we use the 'decltype'
    of our 'cvl::const_param' as the parameter type.

    What happens behind the scenes is that
    the constructor of the type of our
    'cvl::const_param' lifts the user-provided
    function argument into a template parameter
*/
template<auto Number = cvl::const_param<int>>
auto print_even_or_odd(decltype(Number)) -> void {
    /*
        We are able to access the
        value of 'Number' using the
        dereference operator, and
        we can use its value in
        compile-time-only constructs
        such as 'if constexpr'.
    */
    if constexpr (*Number % 2 == 0) {
        std::printf("%d is even!\n", *Number);
    } else {
        std::printf("%d is odd!\n", *Number);
    }
}

/*
    We can even put types into a 'cvl::const_param'
    by accepting a type reflection with 'std::meta::info'.
*/
template<auto Type = cvl::const_param<std::meta::info>>
auto print_size_of(decltype(Type)) -> void {
    std::printf("Size of %s: %zu\n", display_string_of(*Type).data(), size_of(*Type));
}

/*
    We can accept both a 'cvl::const_param'
    and a normal, runtime-only parameter.
*/
template<typename T, auto Count = cvl::const_param<std::size_t>>
auto repeated_array(const T &value, decltype(Count)) -> std::span<const T> {
    /*
        NOTE: Because the 'Count' parameter is
        unique with every call, then this 'static'
        variable will be unique with every call as well.
    */
    static constinit std::array<T, *Count> repeated_values = {};

    std::ranges::fill(repeated_values, value);

    return std::span(repeated_values);
}

int main() {
    print_even_or_odd(1);
    print_even_or_odd(2);
    print_even_or_odd(3);

    print_size_of(^^int);
    print_size_of(^^char);
    print_size_of(^^void *);

    const auto values = repeated_array(42, 5);

    for (const auto value : values) {
        std::printf("Repeated value: %d\n", value);
    }
}
