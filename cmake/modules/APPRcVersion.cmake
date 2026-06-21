set(_VERSION_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR})

include(${PROJECT_SOURCE_DIR}/VERSION.cmake)
include(${PROJECT_SOURCE_DIR}/THEME.cmake)

function(app_add_windows_version_info targetName)
    if(NOT WIN32)
        return()
    endif()

    if(MIRALL_VERSION_BUILD)
        set(APP_RC_VERSION_BUILD ${MIRALL_VERSION_BUILD})
    else()
        set(APP_RC_VERSION_BUILD 0)
    endif()

    get_target_property(TARGET_TYPE ${targetName} TYPE)
    if(${TARGET_TYPE} STREQUAL "EXECUTABLE")
        set(APP_RC_TYPE "VFT_APP")
    elseif(${TARGET_TYPE} STREQUAL "SHARED_LIBRARY" OR ${TARGET_TYPE} STREQUAL "MODULE_LIBRARY")
        set(APP_RC_TYPE "VFT_DLL")
    else()
        # only create version.rc for dll's and executables
        return()
    endif()

    set(APP_RC_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${targetName}-app_rc_version.rc)
    configure_file(${_VERSION_SOURCE_DIR}/version.rc.in ${APP_RC_OUTPUT} @ONLY)

    target_sources(${targetName} PRIVATE ${APP_RC_OUTPUT})
endfunction()
