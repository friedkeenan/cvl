#include <cvl/cvl.hpp>

int main() {
    /*
        A 'cvl::map' models a map-like interface.

        It comes with a few peculiarities:

        Its keys are compared via template-argument-equivalence,
        so there is no hashing involved.

        Its elements cannot be looped over,
        unlike 'std::map' and such. There
        must be a key provided by the user
        in order to access a certain value.

        A value may be inserted
        for a given key only once.
        Values cannot be modified
        once they have been inserted.
    */
    static constexpr cvl::map<char, int> example_map;

    consteval {
        /* We can insert a value for a key using the 'insert' method. */
        example_map.insert('a', 1);
    }

    /* We can access an inserted value using the index operator. */
    static_assert(example_map['a'] == 1);

    consteval {
        /*
            Uncommenting the following line will result in an error.

            This is because we may only insert a value for a given key once.
        */
        // example_map.insert('a', 2);
    }

    consteval {
        /*
            We can avoid the above error by using 'try_insert'.

            It will return a 'bool' indicating
            whether the value was inserted or not.
        */

        /* Here, 'inserted_a' will be false. */
        [[maybe_unused]] const bool inserted_a = example_map.try_insert('a', 2);

        /* Here, 'inserted_b' will be true. */
        [[maybe_unused]] const bool inserted_b = example_map.try_insert('b', 2);
    }

    /* We can see if a given key has a value inserted using the 'contains' method. */
    static_assert(example_map.contains('b'));
    static_assert(not example_map.contains('c'));

    /*
        We can access a value for a given key only if it
        has been inserted by using the 'try_at' method.
    */
    static_assert(example_map.try_at('b').value_or(-1) == 2);
    static_assert(example_map.try_at('c').value_or(-1) == -1);
}
