#include <cstdio>
#include <vector>

#include <cvl/cvl.hpp>

/*
    A 'cvl::list' is a range with somewhat peculiar properties:

    It is a random access range, because its
    elements can be accessed in constant time.

    It is not a contiguous range, because its
    elements don't sit next to each other in memory.

    It is not a sized range, because its
    size cannot be determined in constant time.

    It can only grow in size, and can only
    append elements to the end of its range.

    It is both a view and a borrowed range.
*/

/* A 'cvl::list' may be default-initialized to an empty state. */
constexpr cvl::list<int> example_list;

/* We can check whether a 'cvl::list' is empty with the typical method. */
static_assert(example_list.empty());

/*
    We can also initialize a 'cvl::list'
    to start out with some elements.

    We can also utilize CTAD here.
*/
constexpr cvl::list initializer_list = {1, 2, 3};

/*
    Initializing a 'cvl::list' with any range other than
    'std::initializer_list' requires an explicit constructor call.
*/
constexpr auto vector_list = cvl::list(std::vector{1, 2, 3});

int main() {
    consteval {
        /* We can push a new element to the back of a 'cvl::list'. */
        example_list.push_back(1);
    }

    /* 'example_list' is now no longer empty. */
    static_assert(not example_list.empty());

    consteval {
        /* We can also append a range of values to the back of a 'cvl::list'. */
        example_list.append_range(std::vector{2, 3});
    }

    /* We can access elements with the index operator. */
    static_assert(example_list[0] == 1);
    static_assert(example_list[1] == 2);
    static_assert(example_list[2] == 3);

    /* We can also access the first element with the 'front' method. */
    static_assert(example_list.front() == 1);

    /*
        There is no 'back' method, but there is a
        'try_back' method which returns an optional.

        This method does not work in constant time.
    */
    static_assert(*example_list.try_back() == 3);

    /*
        There is also a similar 'try_at' method.

        This does work in constant time.
    */
    static_assert(*example_list.try_at(1) == 2);

    consteval {
        /*
            During consteval, a 'cvl::list' may
            be iterated over with a 'for' loop.
        */
        for ([[maybe_unused]] const auto &value : example_list) {
            /* ... */
        }
    }

    /*
        At runtime, a 'cvl::list' may be
        iterated over with a 'template for'.
    */
    template for (constexpr auto &Value : example_list) {
        std::printf("Value: %d\n", Value);
    }

    consteval {
        /*
            We can add more elements to a 'cvl::list'
            and then iterate over it again.
        */
        example_list.append_range(std::vector{4, 5, 6});
    }

    template for (constexpr auto &Value : example_list) {
        std::printf("Updated Value: %d\n", Value);
    }
}
