#pragma once

#include <cvl/common.hpp>

namespace cvl::impl {

    /*
        We use friend injection in order to introduce
        our "consteval state", and since friend injection
        operates on function signatures, and thereby
        operates on *types*, we  thenneed to make sure
        that each "state" has its own unique type.

        We accomplish this by creating a "tag" that will
        be used as a template parameter to keep types
        distinct from each other.

        Tagged entities can inherit from 'impl::tagged' and
        conveniently and automatically get a unique tag,
        an object of type 'std::meta::info', which can then
        be passed off to various templates to get at the
        "backend" of a given state.
    */
    struct tagged {
        /* We get the constant of the 'this' pointer as our unique tag. */
        std::meta::info _tag = std::meta::reflect_constant(this);

        /* A helper function to wrap our tag appropriately and send it off to the template. */
        consteval auto _substitute_tag(this tagged self, std::meta::info templ) -> std::meta::info {
            return substitute(templ, {
                std::meta::reflect_constant(self._tag)
            });
        }
    };

}
