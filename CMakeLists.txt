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

set(CVL_HEADERS)

foreach (header
    common.hpp
    const_param.hpp
    cvl.hpp
    defines.hpp
    delayed_init.hpp
    error.hpp
    expand.hpp
    includes.hpp
    list.hpp
    map.hpp
    once_flag.hpp
    undefines.hpp
    util.hpp
    variable.hpp
)
    list(APPEND CVL_HEADERS include/cvl/${header})
endforeach ()

target_sources(cvl
    PUBLIC FILE_SET HEADERS
        BASE_DIRS include
        FILES

        ${CVL_HEADERS}
)

if (${CVL_EXAMPLES})

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
