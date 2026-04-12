#include <cvl/cvl.hpp>

constexpr cvl::once_flag example_flag;

int main() {
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
            You can set a flag as many times as
            you want, but you can't unset a flag.
        */
        example_flag.set();
    }

    /* The flag indeed still reports that it is set. */
    static_assert(example_flag.get());
}
