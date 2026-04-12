#include <cstdio>
#include <cstring>
#include <meta>
#include <string>

#include <cvl/cvl.hpp>

/*
    'cvl:;expand_loop' is conceptually akin
    to a 'template for' expansion statement.

    With it, we can expand out a series
    of statements until we meet some
    arbitrary, user-provided condition
    and break out of the expanded loop.

    So conceptually it is perhaps more
    similar to a 'template while (true)'.
*/

/* We will use this variable later in this example. */
constexpr cvl::variable string_var = std::define_static_string("Initial state.");

int main() {
    /*
        The body for 'cvl::expand_loop' must
        be a templated lambda, which accepts
        a constant 'Loop' parameter.
    */
    cvl::expand_loop([]<auto Loop>() {
        /*
            The 'Loop' parameter has an 'index' method
            which corresponds to which loop iteration
            we are currently executing.
        */
        std::printf("Iteration: %zu\n", Loop.index());

        consteval {
            /*
                The 'Loop' parameter also has a
                'push_break' method which signals
                that the expanded loop should end.

                Here we do that once the
                loop index has reached '2'.
            */
            if (Loop.index() >= 2) {
                Loop.push_break();
            }
        }
    });

    /* We can also use and update our own variables with 'cvl::expand_loop'. */
    cvl::expand_loop([]<auto Loop>() {
        /*
            We pull out the current string, making
            it dependent on the 'Loop' parameter.
        */
        constexpr auto Current = string_var.dependent(Loop);

        std::printf("String %zu: %s\n", Loop.index(), Current);

        consteval {
            /* We accumulate more state with each loop iteration. */
            string_var = std::define_static_string(
                std::string(Current) + " And then more state!"
            );

            /*
                We break out of the loop once our
                string reaches a certain length.
            */
            if (std::strlen(*string_var) > 70) {
                Loop.push_break();
            }
        }
    });

    std::printf("Final string: %s\n", *string_var);
}
