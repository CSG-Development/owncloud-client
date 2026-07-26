if(NOT BINARYCREATOR OR NOT CONFIG OR NOT DATA_DIR OR NOT APP_OUT)
    message(FATAL_ERROR "MakeMaintenanceToolApp: need -DBINARYCREATOR= -DCONFIG= -DDATA_DIR= -DAPP_OUT=")
endif()

if(NOT EXISTS "${BINARYCREATOR}")
    message(FATAL_ERROR "MakeMaintenanceToolApp: binarycreator not found at ${BINARYCREATOR}")
endif()
if(NOT EXISTS "${CONFIG}")
    message(FATAL_ERROR "MakeMaintenanceToolApp: config.xml not found at ${CONFIG}")
endif()

if(NOT CERT)
    set(CERT "-")
endif()

file(REMOVE_RECURSE "${APP_OUT}")
file(MAKE_DIRECTORY "${DATA_DIR}")

message(STATUS "MakeMaintenanceToolApp: creating maintenance tool")
execute_process(
    COMMAND "${BINARYCREATOR}" --config "${CONFIG}" --create-maintenancetool
    WORKING_DIRECTORY "${DATA_DIR}"
    RESULT_VARIABLE _rc)
if(NOT _rc EQUAL 0)
    message(FATAL_ERROR "MakeMaintenanceToolApp: binarycreator --create-maintenancetool failed (rc=${_rc})")
endif()
if(NOT EXISTS "${APP_OUT}")
    message(FATAL_ERROR "MakeMaintenanceToolApp: expected ${APP_OUT} was not produced")
endif()

set(_plist "${APP_OUT}/Contents/Info.plist")

function(mt_set_plist_string key value)
    execute_process(
        COMMAND /usr/libexec/PlistBuddy -c "Set :${key} ${value}" "${_plist}"
        RESULT_VARIABLE _rc OUTPUT_QUIET ERROR_QUIET)
    if(NOT _rc EQUAL 0)
        execute_process(
            COMMAND /usr/libexec/PlistBuddy -c "Add :${key} string ${value}" "${_plist}"
            RESULT_VARIABLE _rc)
        if(NOT _rc EQUAL 0)
            message(FATAL_ERROR "MakeMaintenanceToolApp: cannot set ${key} (rc=${_rc})")
        endif()
    endif()
endfunction()

if(BUNDLE_NAME)
    mt_set_plist_string(CFBundleName "${BUNDLE_NAME}")
    mt_set_plist_string(CFBundleDisplayName "${BUNDLE_NAME}")
endif()

if(BUNDLE_ID)
    mt_set_plist_string(CFBundleIdentifier "${BUNDLE_ID}")
endif()

if(DATA_DIRECTORY)
    mt_set_plist_string(IFWDataDirectory "${DATA_DIRECTORY}")
endif()

if(ICON_SOURCE_APP)
    file(GLOB _icns "${ICON_SOURCE_APP}/Contents/Resources/*.icns")
    if(_icns)
        list(GET _icns 0 _icon)
        get_filename_component(_icon_name "${_icon}" NAME)
        file(COPY "${_icon}" DESTINATION "${APP_OUT}/Contents/Resources")
        message(STATUS "MakeMaintenanceToolApp: using icon ${_icon_name}")
        mt_set_plist_string(CFBundleIconFile "${_icon_name}")
    else()
        message(WARNING "MakeMaintenanceToolApp: no .icns found in ${ICON_SOURCE_APP}/Contents/Resources")
    endif()
endif()

if(CERT STREQUAL "-")
    message(STATUS "MakeMaintenanceToolApp: ad-hoc signing ${APP_OUT}")
    execute_process(COMMAND codesign --force --deep --sign - "${APP_OUT}" RESULT_VARIABLE _rc)
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR "MakeMaintenanceToolApp: ad-hoc codesign failed (rc=${_rc})")
    endif()
else()
    message(STATUS "MakeMaintenanceToolApp: signing ${APP_OUT} with '${CERT}' (hardened runtime)")
    execute_process(
        COMMAND codesign --force --options runtime --timestamp --sign "${CERT}" "${APP_OUT}"
        RESULT_VARIABLE _rc)
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR "MakeMaintenanceToolApp: codesign failed (rc=${_rc})")
    endif()
endif()

execute_process(
    COMMAND codesign --verify --strict --verbose=2 "${APP_OUT}"
    RESULT_VARIABLE _rc)
if(NOT _rc EQUAL 0)
    message(FATAL_ERROR "MakeMaintenanceToolApp: codesign verify failed (rc=${_rc})")
endif()

if(NOTARIZE AND NOT CERT STREQUAL "-")
    if(NOT NOTARY_PROFILE)
        message(FATAL_ERROR "MakeMaintenanceToolApp: NOTARIZE=ON but NOTARY_PROFILE not set")
    endif()
    set(_zip "${APP_OUT}.zip")
    file(REMOVE "${_zip}")
    execute_process(
        COMMAND ditto -c -k --keepParent "${APP_OUT}" "${_zip}"
        RESULT_VARIABLE _rc)
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR "MakeMaintenanceToolApp: ditto zip failed (rc=${_rc})")
    endif()

    message(STATUS "MakeMaintenanceToolApp: notarizing ${_zip}")
    execute_process(
        COMMAND xcrun notarytool submit "${_zip}"
                --keychain-profile "${NOTARY_PROFILE}" --wait
        OUTPUT_VARIABLE _nout
        ERROR_VARIABLE  _nerr
        RESULT_VARIABLE _rc)
    message(STATUS "notarytool:\n${_nout}${_nerr}")

    string(REGEX MATCH "id: ([0-9a-fA-F-]+)" _idmatch "${_nout}")
    set(_sub_id "${CMAKE_MATCH_1}")

    if(NOT _rc EQUAL 0 OR NOT _nout MATCHES "status: Accepted")
        if(_sub_id)
            message(STATUS "MakeMaintenanceToolApp: fetching notarization log for ${_sub_id}")
            execute_process(
                COMMAND xcrun notarytool log "${_sub_id}"
                        --keychain-profile "${NOTARY_PROFILE}"
                OUTPUT_VARIABLE _log ERROR_VARIABLE _logerr)
            message(STATUS "notarytool log:\n${_log}${_logerr}")
        endif()
        message(FATAL_ERROR "MakeMaintenanceToolApp: notarization did not succeed (see log above)")
    endif()

    execute_process(COMMAND xcrun stapler staple "${APP_OUT}" RESULT_VARIABLE _rc)
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR "MakeMaintenanceToolApp: stapler staple failed (rc=${_rc})")
    endif()
    file(REMOVE "${_zip}")
endif()

message(STATUS "MakeMaintenanceToolApp: built ${APP_OUT}")
