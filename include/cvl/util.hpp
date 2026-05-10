#pragma once

#include <cvl/common.hpp>

namespace cvl::util {

    /*
        We use friend injection in order to introduce
        our "consteval state", and since friend injection
        operates on function signatures, and thereby
        operates on *types*, we then need to make sure
        that each "state" has its own unique type.

        We accomplish this by creating a "tag" that will
        be used as a template parameter to keep types
        distinct from each other.

        Tagged entities can include a field of type 'util::tag'
        and conveniently and automatically get a unique tag,
        which can then be passed off to various templates to
        get at the "backend" of a given stateful object.
    */
    struct tag {
        std::meta::info _underlying;

        /*
            A 'util::tag' uses its own address as its underlying tag.

            NOTE: We eagerly turn the address
            into a 'std::meta::info' so that
            we can error on object construction
            if the address is not static, instead
            of waiting until it's used as a tag.
        */
        consteval tag() {
            /*
                I believe that whether an address is static
                is only able to be checked by catching the
                exception thrown by 'std::meta::reflect_constant'.

                That then means that we can only give a better
                diagnostic when exceptions are enabled.
            */
            #if defined(__cpp_exceptions)

            try {
                this->_underlying = std::meta::reflect_constant(this);
            } catch (const std::meta::exception &) {
                /* Rethrow a more helpful exception. */
                CVL_ERROR("A tagged object must have a static address, but this object has a non-static address");
            }

            #else

            this->_underlying = std::meta::reflect_constant(this);

            #endif
        }

        /* A helper function to pass off the tag to a template. */
        consteval auto substitute(this const tag self, std::meta::info templ) -> std::meta::info {
            return std::meta::substitute(templ, {std::meta::reflect_constant(self)});
        }

        template<std::meta::reflection_range R = std::initializer_list<std::meta::info>>
        consteval auto substitute(this const tag self, std::meta::info templ, R &&args) -> std::meta::info {
            auto real_args = std::vector{std::meta::reflect_constant(self)};

            real_args.append_range(std::forward<R>(args));

            return std::meta::substitute(templ, real_args);
        }

        consteval auto operator ==(const tag &) const -> bool = default;
    };

    namespace impl {

        /* Checking 'sizeof' forces the compiler to instantiate a templated type. */
        template<typename T>
        constexpr inline std::size_t ensure_type = sizeof(T);

        /* Getting the address of a function forces the compiler to instantiate a templated function. */
        template<std::meta::info Function>
        constexpr inline bool ensure_function = (&[: Function :] == nullptr);

    }

    /* Ensures that a template substitution gets instantiated by the compiler. */
    consteval auto ensure_instantiation(const std::meta::info entity) -> void {
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

    /* If a type is 'constant_reflectable' then it can be used with 'std::meta::reflect_constant'. */
    template<typename T>
    concept constant_reflectable = (
        std::is_copy_constructible_v<std::decay_t<T>> and

        std::is_structural_v<std::decay_t<T>>
    );

    template<typename T, typename Other>
    concept unqualified_same_as = std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<Other>>;

}
