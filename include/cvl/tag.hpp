#pragma once

#include <cvl/common.hpp>

namespace cvl::impl {

    /*
        We use friend injection in order to introduce
        our "consteval state", and since friend injection
        operates on function signatures, and thereby
        operates on *types*, we  then need to make sure
        that each "state" has its own unique type.

        We accomplish this by creating a "tag" that will
        be used as a template parameter to keep types
        distinct from each other.

        Tagged entities can inherit from 'impl::tagged' and
        conveniently and automatically get a unique tag,
        an object of type 'cvl::impl::tag', which can then
        be passed off to various templates to get at the
        "backend" of a given state.
    */

    /* Forward declare. */
    struct tagged;

    struct tag {
        std::meta::info _tag;

        /*
            NOTE: We eagerly turn the address
            into a 'std::meta::info' so that
            we can error on object construction
            if the address is not static, instead
            of waiting until it's used as a tag.
        */
        consteval explicit tag(const impl::tagged *address) {
            /*
                I believe that whether an address is static
                is only able to be checked by catching the
                exception thrown by 'std::meta::reflect_constant'.

                That then means that we can only give a better
                diagnostic when exceptions are enabled.
            */
            #if defined(__cpp_exceptions)

            try {
                this->_tag = std::meta::reflect_constant(address);
            } catch (const std::meta::exception &) {
                /* Rethrow a more helpful exception. */
                CVL_ERROR("A tagged object must have a static address, but this object has a non-static address");
            }

            #else

            this->_tag = std::meta::reflect_constant(address);

            #endif
        }

        /* A default constructor yielding a "null tag" is helpful for other code. */
        consteval tag() = default;

        consteval auto operator ==(const tag &) const -> bool = default;
    };

    struct tagged {
        /*
            Tagged types operate like a view, like how
            'std::meta::info' and pointer types do.

            They in fact all refer to some other object,
            and operations on one tagged object will be
            reflected by all other tagged objects which
            refer to the same tag.
        */

        /*
            We get the constant of our 'this' pointer as our unique tag.

            This means that all originating-declarations
            (i.e. not copies) of an 'impl::tagged' object
            must have a static address.
        */
        impl::tag _tag = impl::tag(this);

        /* A helper function to wrap our tag appropriately and send it off to the template. */
        consteval auto _substitute_tag(this const tagged self, const std::meta::info templ) -> std::meta::info {
            return substitute(templ, {
                std::meta::reflect_constant(self._tag)
            });
        }

        template<std::meta::reflection_range R = std::initializer_list<std::meta::info>>
        consteval auto _substitute_tag(this const tagged self, const std::meta::info templ, R &&args) -> std::meta::info {
            auto real_args = std::vector{std::meta::reflect_constant(self._tag)};

            real_args.append_range(std::forward<R>(args));

            return substitute(templ, real_args);
        }
    };

}
