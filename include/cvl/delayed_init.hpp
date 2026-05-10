#pragma once

#include <cvl/common.hpp>
#include <cvl/once_flag.hpp>
#include <cvl/util.hpp>

namespace cvl {

    namespace impl {

        /* Tracks whether initialization has happened. */
        template<util::tag>
        constexpr inline cvl::once_flag delayed_init_flag;

        template<util::tag>
        struct delayed_init {
            CVL_DISABLE_FRIEND_WARNING(

                /* Will later have its definition filled in by 'impl::set_delayed_init_value'. */
                friend consteval auto cvl_delayed_init_value(delayed_init) -> auto;

            )
        };

        template<util::tag Tag, auto Value>
        struct set_delayed_init_value {
            CVL_DISABLE_FRIEND_WARNING(

                /* Filling in the function previously declared in 'impl::delayed_init'. */
                friend consteval auto cvl_delayed_init_value(impl::delayed_init<Tag>) -> auto {
                    return Value;
                }

            )

            consteval {
                /* Signal that initialization has taken place. */
                impl::delayed_init_flag<Tag>.set();
            }
        };

        /* Helper template to get the value for the 'delayed_init' tag. */
        template<util::tag Tag>
        constexpr inline auto delayed_init_value = cvl_delayed_init_value(impl::delayed_init<Tag>{});

    }

    template<util::constant_reflectable T>
    struct delayed_init {
        util::tag _tag;

        consteval auto has_value(this const delayed_init self) -> bool {
            const auto flag = extract<cvl::once_flag>(
                self._tag.substitute(^^impl::delayed_init_flag)
            );

            return flag.get();
        }

        template<std::convertible_to<T> Other>
        requires (not util::unqualified_same_as<Other, delayed_init>)
        consteval auto operator =(this const delayed_init &self, Other &&value) -> const delayed_init & {
            if (self.has_value()) {
                CVL_ERROR("Cannot set value of 'cvl::delayed_init' which has already been initialized");
            }

            const auto set_value = self._tag.substitute(^^impl::set_delayed_init_value, {
                std::meta::reflect_constant(
                    static_cast<T>(std::forward<Other>(value))
                )
            });

            util::ensure_instantiation(set_value);

            return self;
        }

        consteval auto operator *(this const delayed_init self) -> const T & {
            if (not self.has_value()) {
                CVL_ERROR("Cannot use value of 'cvl::delayed_init' before it has been initialized");
            }

            return extract<const T &>(
                self._tag.substitute(^^impl::delayed_init_value)
            );
        }

        consteval auto operator ->(this const delayed_init self) -> const T * {
            return std::addressof(*self);
        }

        consteval auto optional(this const delayed_init self) -> std::optional<const T &> {
            if (not self.has_value()) {
                return std::nullopt;
            }

            return *self;
        }
    };

}
