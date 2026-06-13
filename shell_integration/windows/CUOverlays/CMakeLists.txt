add_library(CUOverlays MODULE
    DllMain.cpp
    CUOverlay.cpp
    CUOverlayFactory.cpp
    CUOverlayRegistrationHandler.cpp
    CUOverlay.rc
    CUOverlays.def
)

target_link_libraries(CUOverlays
    CUUtil)

app_add_windows_version_info(CUOverlays)

install(TARGETS CUOverlays
    RUNTIME DESTINATION "${APP_INSTALL_BINDIR}"
    LIBRARY DESTINATION "${APP_INSTALL_BINDIR}"
)
