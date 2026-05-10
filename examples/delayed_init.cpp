#include <cvl/cvl.hpp>

int main() {
    static constexpr cvl::delayed_init<int> int_value;

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
            Uncommenting the following line will result in an error.

            This is because we may only set a 'cvl::delayed_init' once.
        */
        // int_value = 43;
    }

    /*
        We can access the value of a 'cvl::delayed_init' only if
        it has had its value set by using the 'try_value' method.
    */
    static_assert(int_value.try_value().value_or(-1) == 42);
}
