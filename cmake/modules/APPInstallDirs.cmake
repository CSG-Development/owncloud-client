include(GNUInstallDirs)

set(APP_INSTALL_BINDIR "${CMAKE_INSTALL_BINDIR}" CACHE PATH "Executable install directory")
set(APP_INSTALL_LIBDIR "${CMAKE_INSTALL_LIBDIR}" CACHE PATH "Library install directory")
set(APP_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}/${APPLICATION_SHORTNAME}" CACHE PATH "Public header install directory")
set(APP_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake" CACHE PATH "CMake package root install directory")
set(APP_INSTALL_PACKAGEDIR "${APP_INSTALL_CMAKEDIR}/${APPLICATION_SHORTNAME}" CACHE PATH "CMake package install directory")
set(APP_INSTALL_PLUGINDIR "${CMAKE_INSTALL_LIBDIR}/${APPLICATION_SHORTNAME}/plugins" CACHE PATH "Plugin install directory")

set(APP_INSTALL_TARGETS_DEFAULT_ARGS
    RUNTIME DESTINATION "${APP_INSTALL_BINDIR}"
    LIBRARY DESTINATION "${APP_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${APP_INSTALL_LIBDIR}"
    BUNDLE DESTINATION "."
)

function(app_install_target targetName)
    install(TARGETS ${targetName} ${ARGN} ${APP_INSTALL_TARGETS_DEFAULT_ARGS})
endfunction()

function(app_install_plugin targetName)
    install(TARGETS ${targetName}
        RUNTIME DESTINATION "${APP_INSTALL_PLUGINDIR}"
        LIBRARY DESTINATION "${APP_INSTALL_PLUGINDIR}"
    )
endfunction()
