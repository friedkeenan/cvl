#pragma once

#include <cvl/common.hpp>
#include <cvl/list.hpp>
#include <cvl/tag.hpp>
#include <cvl/util.hpp>

namespace cvl {

    namespace impl {

        template<typename From, typename To>
        concept only_explicitly_convertible_to = not std::convertible_to<From, To> and requires {
            static_cast<To>(std::declval<From>());
        };

    }

    template<util::constant_reflectable T>
    requires (not std::is_reference_v<T>)
    struct variable {
        /*
            A 'cvl::variable' is built on top of
            a 'cvl::list', where each element in
            the list is a certain state of the variable.
        */
        cvl::list<T> _states;

        template<std::convertible_to<T> Other>
        requires (not util::qualified_version_of<Other, variable>)
        consteval explicit(false) variable(Other &&other) {
            this->states().push_back(std::forward<Other>(other));
        }

        template<impl::only_explicitly_convertible_to<T> Other>
        requires (not util::qualified_version_of<Other, variable>)
        consteval explicit variable(Other &&other) {
            this->states().push_back(
                static_cast<T>(std::forward<Other>(other))
            );
        }

        /*
            Because we use a 'cvl::list' to update our states,
            we can also just give access to that state list.
        */
        consteval auto states(this const variable self) -> cvl::list<T> {
            return self._states;
        }

        template<std::convertible_to<T> Other>
        requires (not util::qualified_version_of<Other, variable>)
        consteval auto operator =(this const variable &self, Other &&other) -> const variable & {
            self.states().push_back(std::forward<Other>(other));

            return self;
        }

        consteval auto operator *(this const variable self) -> const T & {
            /* The last state is our latest state. */
            const auto back = self.states().try_back();

            /* There will always be at least one element in the state list. */
            return *back;
        }

        consteval auto operator ->(this const variable self) -> const T * {
            return std::addressof(*self);
        }

        /* Used to explicitly access a variable as a dependent expression. */
        consteval auto dependent(this const variable self, auto &&) -> const T & {
            return *self;
        }
    };

    template<typename T>
    variable(T &&) -> variable<std::decay_t<T>>;

}
