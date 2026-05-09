cmake_minimum_required(VERSION 3.25)

project(cvl VERSION 0.1.0 LANGUAGES CXX)

if (${PROJECT_IS_TOP_LEVEL} AND NOT DEFINED CMAKE_COLOR_DIAGNOSTICS)

    set(CMAKE_COLOR_DIAGNOSTICS ON)

endif ()

option(CVL_EXAMPLES "Generate the examples target." ${PROJECT_IS_TOP_LEVEL})
option(CVL_INSTALL  "Generate the install target."  ${PROJECT_IS_TOP_LEVEL})

add_library(cvl INTERFACE)
add_library(cvl::cvl ALIAS cvl)

target_compile_features(cvl INTERFACE
    cxx_std_26
)

target_compile_options(cvl INTERFACE -freflection)

file(GLOB CVL_HEADERS ${CMAKE_CURRENT_LIST_DIR}/include/cvl/*.hpp)

target_sources(cvl
    PUBLIC FILE_SET HEADERS
        BASE_DIRS include
        FILES

        ${CVL_HEADERS}
)

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

if (${CVL_INSTALL})

    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(TARGETS cvl
        EXPORT cvl_targets

        FILE_SET HEADERS

        COMPONENT Development
    )

    install(EXPORT cvl_targets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cvl

        NAMESPACE cvl::

        FILE cvl-config.cmake

        COMPONENT Development
    )

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/cvl-config-version.cmake

        COMPATIBILITY ExactVersion
    )

    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/cvl-config-version.cmake

        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cvl
    )

    if (${CMAKE_VERSION} VERSION_GREATER_EQUAL 4.3.0)

        install(PACKAGE_INFO cvl
            EXPORT cvl_targets

            COMPONENT Development

            LICENSE ISC
        )

    endif ()

    install(
        FILES LICENSE README.md

        TYPE DOC

        COMPONENT Documentation
    )

endif ()
