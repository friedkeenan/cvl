#include <cstdio>

#include <cvl/cvl.hpp>

/*
    A 'cvl::variable' models a mutable variable.

    It must be initialized with a given
    value, and afterwards we can modify
    it an arbitrary number of times.
*/
constexpr cvl::variable<int> example_var = 1;

/* We can also utilize CTAD here. */
constexpr cvl::variable deduced_var = 2;

int main() {
    /* We can access a 'cvl::variable' with the dereference operator. */
    static_assert(*example_var == 1);

    consteval {
        /* We can modify a 'cvl::variable' by assigning to it. */
        example_var = 2;
    }

    /* The value has indeed been updated. */
    static_assert(*example_var == 2);

    /* We can use and update a 'cvl::variable' in an expansion statement. */
    template for (constexpr int Value : {1, 2, 3}) {
        /*
            With the cvl library, sometimes the compiler
            will try to outsmart us and evaluate certain
            things at inopportune times for what we intend.

            For this problem, 'cvl::variable' has a
            helper method 'dependent' which allows
            us to explicitly get its value through
            a dependent expression.
        */

        /*
            Here we explicitly tell the compiler that
            the value of 'Current' depends on our expanded
            'Value' variable, which is a dependent name.

            This will then force the compiler
            to delay evaluating the expression
            until it has bound the dependent name
            'Value', achieving our desired behavior.

            The actual value of 'Value' however
            is not actually used in the expression.
            Its name is only there to massage the compiler.
        */
        constexpr int Current = example_var.dependent(Value);

        std::printf("Current: %d\n", Current);

        consteval {
            /* Update 'example_var' while in the 'template for'. */
            example_var = Current + Value;
        }
    }

    std::printf("Final: %d\n", *example_var);

    /*
        We can also use the 'states' method to
        get a 'cvl::list' of all the states
        which a 'cvl::variable' has had.
    */
    template for (constexpr auto State : example_var.states()) {
        std::printf("Example state: %d\n", State);
    }
}
