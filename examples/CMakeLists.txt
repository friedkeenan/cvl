add_custom_target(cvl_examples)

function (cvl_example target)

    add_executable(${target}_example)

    target_sources(${target}_example PRIVATE ${target}.cpp)

    set_target_properties(${target}_example PROPERTIES
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

    add_dependencies(cvl_examples ${target}_example)

endfunction ()

cvl_example(const_param)
cvl_example(delayed_init)
cvl_example(expand_loop)
cvl_example(list)
cvl_example(map)
cvl_example(once_flag)
cvl_example(tag)
cvl_example(variable)
