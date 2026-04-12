#include <cvl/cvl.hpp>

constexpr cvl::delayed_init<int> int_value;

int main() {
    /* A 'cvl::delayed_init' begins with no value. */
    static_assert(not int_value.has_value());

    consteval {
        /* Update 'int_value' to now have a value of '42'. */
        int_value = 42;
    }

    /* 'int_value' now reports that it has a value. */
    static_assert(int_value.has_value());

    /* We can get its value with the dereference operator. */
    static_assert(*int_value == 42);

    consteval {
        /*
            Uncommenting this line will result in an error.

            You may only set a 'cvl::delayed_init' once.
        */
        // int_value = 43;
    }

    /* A 'cvl::delayed_init' can be mapped to a 'std::optional'. */
    static_assert(int_value.optional().value_or(-1) == 42);
}
