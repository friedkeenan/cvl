#pragma once

#include <cvl/common.hpp>
#include <cvl/once_flag.hpp>
#include <cvl/util.hpp>

namespace cvl {

    namespace impl {

        struct loop_controller {
            cvl::once_flag _break_flag;

            /*
                Each loop iteration must be distinct,
                so we track the index of the loop.
            */
            std::size_t _index;

            consteval auto index(this const loop_controller self) -> std::size_t {
                return self._index;
            }

            consteval auto push_break(this const loop_controller self) -> void {
                self._break_flag.set();
            }
        };

        template<typename Body, std::meta::info... CallOperators>
        struct replicator {
            static constexpr auto execute(Body &body) -> void {
                template for (constexpr auto CallOperator : {CallOperators...}) {
                    body.[: CallOperator :]();
                }
            }
        };

        /* TODO: Put in function bodies (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=124824). */
        template<typename Body>
        constexpr inline cvl::once_flag break_flag;

        template<typename Body>
        consteval auto expand_loop() -> std::meta::info {
            auto args = std::vector{^^Body};

            std::size_t index = 0;

            while (true) {
                const auto controller = impl::loop_controller{impl::break_flag<Body>, index};

                const auto call_operator = substitute(^^Body::template operator (), {
                    std::meta::reflect_constant(controller)
                });

                cvl::util::ensure_instantiation(call_operator);

                args.push_back(std::meta::reflect_constant(call_operator));

                if (impl::break_flag<Body>.get()) {
                    break;
                }

                ++index;
            }

            return substitute(^^impl::replicator, args);
        }

    }

    /*
        NOTE: We call 'impl:;expand_loop' in the
        function interface to force the compiler
        to update state and such appropriately.

        TODO: It would be nice to have
        some requirements on 'Body' but
        that doesn't seem to work currently.
    */
    template<typename Body, typename Replicator = [: impl::expand_loop<std::remove_reference_t<Body>>() :]>
    constexpr auto expand_loop(Body &&body) -> void {
        Replicator::execute(body);
    }

    template<typename Body>
    constexpr auto expand_loop(Body &) -> void = delete("Loop bodies must be rvalues");

}
