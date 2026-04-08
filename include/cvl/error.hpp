#pragma once

#include <cvl/common.hpp>

namespace cvl {

    /*
        In order to support environments both
        with and without exceptions, we use
        slightly different erroring schemes
        for each, and conjoin their interface
        by using a macro defined in 'defines.hpp'.

        You apparently are effectively disallowed
        from inheriting from 'std::meta::exception'
        because you can't downgrade its destructor
        from 'consteval' to 'constexpr' but at
        the same time you can't define a 'consteval'
        destructor yourself.

        So in the exception-supported environment,
        we just use 'std::meta::exception'.

        TODO: See if there's a better way than the above.
    */
    #if not defined(__cpp_exceptions)

    /*
        In the without-exceptions environment,
        we instead call a non-consteval function
        during constant evaluation, which will
        cause a compiler error.

        This non-consteval function has the
        description of the error passed to
        its template parameters, so we can
        still do our best to give relevant
        information to the user.
    */

    namespace impl {

        template<const char *What, std::meta::info From>
        auto reached_error() -> void;

    }

    #endif

}
