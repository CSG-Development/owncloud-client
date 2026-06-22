include(APPApplyCommonSettings)

function(add_vfs_plugin)
    set(options "")
    set(oneValueArgs NAME)
    set(multiValueArgs SRC LIBS)
    cmake_parse_arguments(__PLUGIN "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_library(vfs_${__PLUGIN_NAME} MODULE
        ${__PLUGIN_SRC}
    )
    app_apply_common_target_settings(vfs_${__PLUGIN_NAME})


    set_target_properties(vfs_${__PLUGIN_NAME} PROPERTIES OUTPUT_NAME "${synclib_NAME}_vfs_${__PLUGIN_NAME}")

    target_link_libraries(vfs_${__PLUGIN_NAME}
        libsync
        ${__PLUGIN_LIBS}
    )
    if(APPLE)
        set_target_properties(vfs_${__PLUGIN_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "$<TARGET_FILE_DIR:PersonalCloud>/../PlugIns/")
        if(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
            add_custom_command(TARGET vfs_${__PLUGIN_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND}
                ARGS -E create_symlink "$<TARGET_FILE:vfs_${__PLUGIN_NAME}>" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<TARGET_FILE_NAME:vfs_${__PLUGIN_NAME}>" MAIN_DEPENDENCY "vfs_${__PLUGIN_NAME}")
        endif()

    else()
        app_install_plugin(vfs_${__PLUGIN_NAME})
    endif()

    if (TARGET PersonalCloud)
        add_dependencies(PersonalCloud vfs_${__PLUGIN_NAME})
    endif()
    if (TARGET Cmd)
        add_dependencies(Cmd vfs_${__PLUGIN_NAME})
    endif()
endfunction()
