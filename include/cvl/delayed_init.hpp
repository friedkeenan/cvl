#pragma once

#include <cvl/common.hpp>
#include <cvl/tag.hpp>
#include <cvl/once_flag.hpp>

namespace cvl {

    namespace impl {

        template<std::meta::info Tag>
        constexpr inline cvl::once_flag delayed_init_flag;

        template<std::meta::info Tag>
        struct delayed_init {
            CVL_PUSH_DISABLE_FRIEND_WARNING

            friend consteval auto cvl_delayed_init_value(delayed_init) -> auto;

            CVL_POP_DISABLE_FRIEND_WARNING
        };

        template<std::meta::info Tag, auto Value>
        struct set_delayed_init_value {
            CVL_PUSH_DISABLE_FRIEND_WARNING

            friend consteval auto cvl_delayed_init_value(impl::delayed_init<Tag>) -> auto {
                return Value;
            }

            CVL_POP_DISABLE_FRIEND_WARNING

            consteval {
                impl::delayed_init_flag<Tag>.set();
            }
        };

        template<std::meta::info Tag>
        constexpr inline auto delayed_init_value = cvl_delayed_init_value(impl::delayed_init<Tag>{});

    }

    /* TODO: Add requirements to type for 'std::meta::reflect_constant'. */
    template<typename T>
    struct delayed_init : impl::tagged {
        consteval auto is_initialized(this const delayed_init &self) -> bool {
            const auto &flag = extract<const cvl::once_flag &>(
                self._substitute_tag(^^impl::delayed_init_flag)
            );

            return flag.get();
        }

        consteval auto operator =(this const delayed_init &self, const T &value) -> const delayed_init & {
            /* TODO: Error if initialized. */

            const auto set_value = self._substitute_tag(^^impl::set_delayed_init_value, {
                std::meta::reflect_constant(value)
            });

            util::ensure_instantiation(set_value);

            return self;
        }

        consteval auto operator *(this const delayed_init &self) -> const T & {
            /* TODO: Error if not initialized. */

            return extract<const T &>(
                self._substitute_tag(^^impl::delayed_init_value)
            );
        }
    };

}
