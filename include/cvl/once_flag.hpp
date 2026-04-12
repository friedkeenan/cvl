#pragma once

#include <cvl/common.hpp>
#include <cvl/tag.hpp>
#include <cvl/util.hpp>

namespace cvl {

    namespace impl {

        template<std::meta::info Tag>
        struct once_flag {
            CVL_DISABLE_FRIEND_WARNING(

                /*
                    This function returns a reflection of itself,
                    which is seemingly the only way you can get a
                    reflection of a friend function such as this one.

                    Later, 'impl::set_once_flag' will redeclare this
                    friend function, and will name the 'once_flag'
                    parameter, which we do not do here.

                    When that redeclaratiom occurs, the compiler will
                    then update whether that parameter has an identifier,
                    and that will change the result of 'std::meta::has_identifier'.

                    We can then check that to see whether
                    'impl::set_once_flag' has been instantiated.
                */
                friend consteval auto cvl_track_once_flag_set(once_flag) -> std::meta::info {
                    return std::meta::current_function();
                }

            )
        };

        template<std::meta::info Tag>
        struct set_once_flag {
            CVL_DISABLE_FRIEND_WARNING(

                /* Redeclare friend function, naming the parameter. */
                friend consteval auto cvl_track_once_flag_set(impl::once_flag<Tag> injected_name) -> std::meta::info;

            )
        };

        /* A helper template for us to call the friend function. */
        template<std::meta::info Tag>
        constexpr inline std::meta::info once_flag_tracker = cvl_track_once_flag_set(impl::once_flag<Tag>{});

    }

    struct once_flag : impl::tagged {
        consteval auto _tracker(this const once_flag self) -> std::meta::info {
            return extract<std::meta::info>(
                self._substitute_tag(^^impl::once_flag_tracker)
            );
        }

        consteval auto get(this const once_flag self) -> bool {
            const auto tracker = self._tracker();

            /* If the flag has been set, then the parameter will have an identifier. */
            return has_identifier(
                parameters_of(tracker)[0]
            );
        }

        consteval auto set(this const once_flag self) -> void {
            if (self.get()) {
                return;
            }

            const auto set_flag = self._substitute_tag(^^impl::set_once_flag);

            util::ensure_instantiation(set_flag);
        }
    };

}
