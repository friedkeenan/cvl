#pragma once

#include <cvl/common.hpp>
#include <cvl/delayed_init.hpp>
#include <cvl/util.hpp>

namespace cvl {

    namespace impl {

        template<typename Tag, typename T>
        struct const_param {
            static constexpr cvl::delayed_init<T> _value_holder = {};

            consteval const_param() = default;

            template<std::convertible_to<T> Other>
            requires (not cvl::util::unqualified_same_as<Other, const_param>)
            consteval explicit(false) const_param(Other &&other) {
                /*
                    The compiler may call our constructor multiple times.

                    If that happens, we just return early.
                */
                if (_value_holder.has_value()) {
                    return;
                }

                _value_holder = std::forward<Other>(other);
            }

            consteval auto operator *(this const_param) -> const T & {
                if (not _value_holder.has_value()) {
                    CVL_ERROR("Cannot get value of 'cvl::const_param' before it's been passed a value");
                }

                return *_value_holder;
            }

            consteval auto operator ->(this const const_param self) -> const T * {
                return std::addressof(*self);
            }
        };

    }

    /*
        NOTE: We seemingly can't use the
        'cvl::util::tag' trick to uniquely
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
    template<util::constant_reflectable T, typename Tag = decltype([]{})>
    constexpr inline auto const_param = impl::const_param<Tag, T>{};

}
