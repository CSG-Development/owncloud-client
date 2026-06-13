if(WITH_CRASHREPORTER)
    find_package(CrashReporterQt REQUIRED)
endif()

find_package(Qt6Keychain 0.13 REQUIRED)

add_subdirectory(resources)
add_subdirectory(csync)
add_subdirectory(libsync)
if (BUILD_GUI)
    add_subdirectory(gui)
endif()
add_subdirectory(cmd)

if (WITH_CRASHREPORTER)
    add_subdirectory(crashreporter)
endif()

add_subdirectory(plugins)
add_subdirectory(telemetry)

install(EXPORT ${APPLICATION_SHORTNAME}Config DESTINATION "${APP_INSTALL_PACKAGEDIR}" NAMESPACE PersonalCloud::)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_SHORTNAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_SHORTNAME}ConfigVersion.cmake" DESTINATION "${APP_INSTALL_PACKAGEDIR}")
