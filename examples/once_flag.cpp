#include <cvl/cvl.hpp>

int main() {
    static constexpr cvl::once_flag example_flag;

    /* A 'cvl::once_flag' begins as unset. */
    static_assert(not example_flag.get());

    consteval {
        /* Update the flag's state to now be set. */
        example_flag.set();
    }

    /* Now the flag reports that it is set. */
    static_assert(example_flag.get());

    consteval {
        /*
            We can set a flag as many times as
            we want, but we can't unset a flag.
        */
        example_flag.set();
    }

    /* The flag indeed still reports that it is set. */
    static_assert(example_flag.get());
}
