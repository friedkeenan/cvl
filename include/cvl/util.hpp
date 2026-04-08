#pragma once

#include <cvl/common.hpp>

namespace cvl::util {

    namespace impl {

        /* Checking 'sizeof' forces the compiler to instantiate a templated type. */
        template<typename T>
        constexpr inline std::size_t ensure_type = sizeof(T);

        /* Getting the address of a function forces the compiler to instantiate a templated function. */
        template<std::meta::info Function>
        constexpr inline bool ensure_function = (&[: Function :] == nullptr);

    }

    /* Ensures that a template substitution gets instantiated by the compiler. */
    consteval auto ensure_instantiation(std::meta::info entity) -> void {
        if (is_type(entity)) {
            (void) extract<std::size_t>(substitute(
                ^^impl::ensure_type, {entity}
            ));

            return;
        }

        if (not is_function(entity)) {
            return;
        }

        (void) extract<bool>(substitute(
            ^^impl::ensure_function, {std::meta::reflect_constant(entity)}
        ));
    }

    template<typename T, typename Other>
    concept qualified_version_of = std::same_as<std::remove_cvref_t<T>, Other>;

}
