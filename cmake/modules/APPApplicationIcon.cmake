function(add_application_icon OUT_VAR)
    set(app_icon_sources)

    if(WIN32)
        set(icon_path "${CMAKE_SOURCE_DIR}/admin/win/msi/gui/personalcloud.ico")
        if(EXISTS "${icon_path}")
            set(rc_path "${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_ICON_NAME}-app-icon.rc")
            set(APP_APPLICATION_ICON_PATH "${icon_path}")
            configure_file("${CMAKE_SOURCE_DIR}/cmake/modules/app-icon.rc.in" "${rc_path}" @ONLY)
            list(APPEND app_icon_sources "${rc_path}")
        else()
            message(WARNING "Windows application icon not found: ${icon_path}")
        endif()
    elseif(APPLE)
        set(APP_APPLICATION_ICON_ICNS "" CACHE FILEPATH "Prepared or generated macOS application .icns file")
        set(icon_path "${APP_APPLICATION_ICON_ICNS}")
        if(icon_path)
            get_filename_component(icon_name "${icon_path}" NAME)
            set_source_files_properties("${icon_path}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
            set(MACOSX_BUNDLE_ICON_FILE "${icon_name}" PARENT_SCOPE)
            list(APPEND app_icon_sources "${icon_path}")
        else()
            set(prepared_icon_path "${OEM_THEME_DIR}/theme/colored/${APPLICATION_ICON_NAME}.icns")
            set(generated_icon_path "${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_ICON_NAME}.icns")

            if(EXISTS "${prepared_icon_path}")
                set(icon_path "${prepared_icon_path}")
                get_filename_component(icon_name "${icon_path}" NAME)
                set_source_files_properties("${icon_path}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
                set(MACOSX_BUNDLE_ICON_FILE "${icon_name}" PARENT_SCOPE)
                list(APPEND app_icon_sources "${icon_path}")
            else()
                find_program(ICONUTIL_EXECUTABLE iconutil)
                if(ICONUTIL_EXECUTABLE)
                    set(iconset_dir "${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_ICON_NAME}.iconset")
                    set(icon_sources)
                    set(icon_copy_commands)
                    foreach(size IN ITEMS 16 32 128 256 512)
                        set(source_icon "${OEM_THEME_DIR}/theme/colored/${size}-${APPLICATION_ICON_NAME}-icon.png")
                        if(EXISTS "${source_icon}")
                            list(APPEND icon_sources "${source_icon}")
                            list(APPEND icon_copy_commands
                                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${source_icon}" "${iconset_dir}/icon_${size}x${size}.png"
                            )
                        endif()

                        math(EXPR retina_size "${size} * 2")
                        set(source_icon "${OEM_THEME_DIR}/theme/colored/${retina_size}-${APPLICATION_ICON_NAME}-icon.png")
                        if(EXISTS "${source_icon}")
                            list(APPEND icon_sources "${source_icon}")
                            list(APPEND icon_copy_commands
                                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${source_icon}" "${iconset_dir}/icon_${size}x${size}@2x.png"
                            )
                        endif()
                    endforeach()

                    if(icon_sources)
                        add_custom_command(
                            OUTPUT "${generated_icon_path}"
                            COMMAND ${CMAKE_COMMAND} -E rm -rf "${iconset_dir}"
                            COMMAND ${CMAKE_COMMAND} -E make_directory "${iconset_dir}"
                            ${icon_copy_commands}
                            COMMAND "${ICONUTIL_EXECUTABLE}" -c icns "${iconset_dir}" -o "${generated_icon_path}"
                            DEPENDS ${icon_sources}
                            VERBATIM
                        )
                        get_filename_component(icon_name "${generated_icon_path}" NAME)
                        set_source_files_properties("${generated_icon_path}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
                        set(MACOSX_BUNDLE_ICON_FILE "${icon_name}" PARENT_SCOPE)
                        list(APPEND app_icon_sources "${generated_icon_path}")
                    else()
                        message(STATUS "macOS application icon generation placeholder: no theme PNG icons found")
                    endif()
                else()
                    message(STATUS "macOS application icon generation placeholder: iconutil not found")
                endif()
            endif()
        endif()
    endif()

    set(${OUT_VAR} ${app_icon_sources} PARENT_SCOPE)
endfunction()
