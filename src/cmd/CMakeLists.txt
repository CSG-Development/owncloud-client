set (CMD_EXECUTABLE "Cmd")
add_executable(${CMD_EXECUTABLE}
    cmd.cpp
    httpcredentialstext.cpp
)
set_target_properties(${CMD_EXECUTABLE} PROPERTIES OUTPUT_NAME "${APPLICATION_EXECUTABLE} Cmd")
ecm_mark_nongui_executable(${CMD_EXECUTABLE})

target_link_libraries(${CMD_EXECUTABLE} csync libsync Qt::Core Qt::Network)
apply_common_target_settings(${CMD_EXECUTABLE})

if(APPLE)
    set_target_properties(${CMD_EXECUTABLE} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "$<TARGET_FILE_DIR:curator>")
else()
    install(TARGETS ${CMD_EXECUTABLE} ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
endif()

if(UNIX AND NOT APPLE)
    configure_file(${CMAKE_SOURCE_DIR}/curatorcmd.desktop.in "${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_EXECUTABLE} Cmd.desktop")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_EXECUTABLE} Cmd.desktop" DESTINATION ${KDE_INSTALL_DATADIR}/applications)
endif()
