#include <cstddef>
#include <meta>

#include <cvl/cvl.hpp>

/*
    A 'cvl::util::tag' is used when a given
    instance of some object needs its own
    unique family of template instantiations.

    That sort of usecase can often arise when
    building your own features on top of the
    the features that 'cvl' provides.

    In this example, we'll look at one such
    usecase: Building a 'counter' class on
    top of 'cvl::once_flag'.
*/

/*
    A 'cvl::util::tag' is dispatched to
    a given template, which will result
    in a unique template instantiation
    for each tag.

    Here, we're dispatching our tags to
    a variable template, along with a
    'std::size_t N' template parameter.

    This template will yield an
    initially-unset 'cvl::once_flag',
    which critically will be unique
    for each tag and 'N' given to it.

    These flags will be used by our
    'counter' class to track how far
    each counter has counted.

    So the "N-th" flag will be set if
    our counter has counted past 'N',
    and will be unset if our counter
    is at or has not yet reached 'N'.
*/
template<cvl::util::tag, std::size_t N>
constexpr inline cvl::once_flag nth_counter_flag;

struct counter {
    /*
        The way that we initially
        retrieve a 'cvl::util::tag'
        is by including it as a
        subobject of our class.

        That will then cause each
        originating-declaration
        (i.e. not copies) of our
        class to be given a unique
        tag for that declaration.

        Copies that are made from
        that originating-declaration
        will have the same tag,
        and therefore will interact
        with the same template
        instantiations, and changes
        to the copy will as well
        affect the original declaration,
        as well as any other copies.
    */
    cvl::util::tag tag;

    /* We'll define a helper method to get the n-th flag. */
    consteval auto nth_flag(std::size_t n) const -> cvl::once_flag {
        /*
            'cvl::util::tag' provides a 'substitute'
            method in order to dispatch it to a template.

            It will place the tag as the first
            template argument, with the other
            provided arguments placed after the tag.

            It returns a reflection of the resulting
            template, as 'std::meta::substitute' does.
        */
        const auto flag_reflection = this->tag.substitute(
            ^^nth_counter_flag, {std::meta::reflect_constant(n)}
        );

        /*
            In order to get the flag out
            of the reflection, we can just
            extract the 'cvl::once_flag'.
        */
        return extract<cvl::once_flag>(flag_reflection);
    }

    /* This method retrieves the current value of the counter. */
    consteval auto current() const -> std::size_t {
        /* Here, we'll loop through each flag until we find an unset one. */

        std::size_t n = 0;

        while (true) {
            const auto flag = this->nth_flag(n);

            /*
                If the flag is not set, then we've
                reached our current value for 'n'.
            */
            if (not flag.get()) {
                return n;
            }

            /* Otherwise if the flag is set, increment and continue. */
            n += 1;
        }
    }

    /*
        This method increments the counter.

        Note that even though it is a
        mutating operation, the method
        accepts a 'const' counter.
    */
    consteval auto increment() const -> void {
        /*
            Here, we can just get the flag for
            our current value and then set it.

            Setting the "current flag" will
            then cause the 'current' method
            to go one 'n' further, effectively
            incrementing our counter's value.
        */

        const auto current_flag = this->nth_flag(this->current());

        current_flag.set();
    }
};

int main() {
    /* Finally we can put our 'counter' class to use. */
    static constexpr counter example_counter;

    /* A counter starts at zero. */
    static_assert(example_counter.current() == 0);

    consteval {
        /* We can increment the counter with our 'increment' method. */
        example_counter.increment();
    }

    /* Now our counter has incremented its value. */
    static_assert(example_counter.current() == 1);

    /*
        Uncommenting the following line
        will result in an error.

        This is because every
        originating-delcaration
        of an object containing
        a 'cvl::util::tag' must
        have a static storage
        duration. This declaration
        is actually not static,
        however, and actually
        lives on the stack.
    */
    // constexpr counter bad_counter;

    /* We can declare a separate counter. */
    static constexpr counter other_counter;

    /*
        This other counter has its own value,
        distinct from our earlier counter.
    */
    static_assert(other_counter.current() == 0);

    /* We can take a copy of a given counter. */
    static constexpr auto copied_counter = other_counter;

    consteval {
        /* If we increment that copied counter... */
        copied_counter.increment();
    }

    /* ...Then that will affect the original declaration too. */
    static_assert(other_counter.current()  == 1);
    static_assert(copied_counter.current() == 1);
}
