cmake_minimum_required(VERSION 3.25)

project(cvl LANGUAGES CXX)

if (${PROJECT_IS_TOP_LEVEL} AND NOT DEFINED CMAKE_COLOR_DIAGNOSTICS)

    set(CMAKE_COLOR_DIAGNOSTICS ON)

endif ()

option(CVL_EXAMPLES "Generate the examples target." ${PROJECT_IS_TOP_LEVEL})

add_library(cvl INTERFACE)
add_library(cvl::cvl ALIAS cvl)

target_compile_features(cvl INTERFACE
    cxx_std_26
)

target_sources(cvl
    PUBLIC FILE_SET HEADERS
        BASE_DIRS include
        FILES
        include/cvl/cvl.hpp
)

target_compile_options(cvl INTERFACE -freflection)

if (${CVL_EXAMPLES})

    add_custom_target(examples)

    function (add_example target)

        add_executable(${target}_example)

        # NOTE: This is called within the 'examples' directory.
        target_sources(${target}_example PRIVATE ${target}.cpp)

        set_target_properties(${target}_example PROPERTIES
            EXCLUDE_FROM_ALL ON

            OUTPUT_NAME ${target}
        )

        target_compile_options(${target}_example PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion

            -Werror
        )

        target_link_libraries(${target}_example PRIVATE cvl::cvl)

        add_dependencies(examples ${target}_example)

    endfunction ()

    add_subdirectory(examples)

endif ()
