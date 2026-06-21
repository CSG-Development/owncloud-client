set (CMD_EXECUTABLE "Cmd")
add_executable(${CMD_EXECUTABLE}
    cmd.cpp
    httpcredentialstext.cpp
)

set_target_properties(${CMD_EXECUTABLE} PROPERTIES OUTPUT_NAME "${APPLICATION_EXECUTABLE}Cmd")

target_link_libraries(${CMD_EXECUTABLE} csync libsync Qt::Core Qt::Network)
app_apply_common_target_settings(${CMD_EXECUTABLE})

if(APPLE)
    set_target_properties(${CMD_EXECUTABLE} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "$<TARGET_FILE_DIR:PersonalCloud>")
else()
    app_install_target(${CMD_EXECUTABLE})
endif()
