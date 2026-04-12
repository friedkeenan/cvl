#pragma once

#include <cvl/common.hpp>
#include <cvl/delayed_init.hpp>
#include <cvl/util.hpp>

namespace cvl {

    namespace impl {

        template<typename Tag, typename T>
        constexpr inline cvl::delayed_init<T> const_param_value;

        template<typename Tag, typename T>
        struct const_param {
            consteval const_param() = default;

            template<std::convertible_to<T> Other>
            requires (not cvl::util::qualified_version_of<Other, const_param>)
            consteval explicit(false) const_param(Other &&other) {
                if (impl::const_param_value<Tag, T>.has_value()) {
                    CVL_ERROR("Cannot set value of 'cvl::const_param' which already has its value set");
                }

                impl::const_param_value<Tag, T> = std::forward<Other>(other);
            }

            consteval auto operator *(this const_param) -> const T & {
                if (not impl::const_param_value<Tag, T>.has_value()) {
                    CVL_ERROR("Cannot get value of 'cvl::const_param' before it's been passed a value");
                }

                return *impl::const_param_value<Tag, T>;
            }

            consteval auto operator ->(this const const_param self) -> const T * {
                return std::addressof(*self);
            }
        };

    }

    /*
        NOTE: We seemingly can't use the
        'cvl::impl::tagged' trick to uniquely
        tag a template parameter, so we fall
        back to using a lambda type as our tag.

        Having a default template parameter which
        doesn't have a consistent value may be
        an ODR violation however: https://stackoverflow.com/a/79289828

        If it is, then the compiler does not *seem* to care.

        But that is the primary reason why
        I am unsure about this utility.

        And I'm not sure if the constructor of
        'impl::const_param' *must* be evaluated
        before instantiating the function body.
    */
    template<typename T, typename Tag = decltype([]{})>
    constexpr inline auto const_param = impl::const_param<Tag, T>{};

}
