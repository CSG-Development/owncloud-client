if(UNIT_TESTING)
    message(DEPRECATION "Setting UNIT_TESTING is deprecated please use BUILD_TESTING")
    set(BUILD_TESTING TRUE)
endif()

option(NO_MSG_HANDLER "Don't redirect QDebug outputs to the log window/file" OFF)
option(BUILD_SHELL_INTEGRATION "BUILD_SHELL_INTEGRATION" ON)
option(BUILD_GUI "BUILD_GUI" ON)
option(WITH_AUTO_UPDATER "Build the legacy Sparkle/OCUpdater auto-update client" OFF)
option(WITH_IFW_UPDATER "Build the QtIFW/MaintenanceTool-based auto-update client, independent of the legacy WITH_AUTO_UPDATER path" OFF)

if(WITH_AUTO_UPDATER AND WITH_IFW_UPDATER)
    message(FATAL_ERROR "WITH_AUTO_UPDATER (legacy Sparkle/OCUpdater) and WITH_IFW_UPDATER (QtIFW MaintenanceTool) are mutually exclusive auto-update mechanisms and must not both be enabled.")
endif()
option(FORCE_ASSERTS "FORCE_ASSERTS" OFF)
option(WITH_CAP_CORE "WITH_CAP_CORE" OFF)
option(BUILD_INSTALLER "Build the Qt Installer Framework installer (target: installer)" OFF)

set(VIRTUAL_FILE_SYSTEM_PLUGINS off suffix CACHE STRING "Name of internal plugin in src/libsync/vfs or the locations of virtual file plugins")

if(APPLE)
    set(SOCKETAPI_TEAM_IDENTIFIER_PREFIX "" CACHE STRING "SocketApi prefix (including a following dot) that must match the codesign key's TeamIdentifier/Organizational Unit")
endif()
