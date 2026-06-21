add_library(CUContextMenu MODULE
    dllmain.cpp
    CUClientInterface.cpp
    CUContextMenu.cpp
    CUContextMenuFactory.cpp
    CUContextMenuRegHandler.cpp
    CUContextMenu.def
)

target_link_libraries(CUContextMenu
                      CUUtil
                      crypt32
                      gdiplus
                      nlohmann_json::nlohmann_json
)
target_compile_definitions(CUContextMenu PRIVATE JSON_NOEXCEPTION)
app_add_windows_version_info(CUContextMenu)

install(TARGETS CUContextMenu
    RUNTIME DESTINATION "${APP_INSTALL_BINDIR}"
    LIBRARY DESTINATION "${APP_INSTALL_BINDIR}"
)
