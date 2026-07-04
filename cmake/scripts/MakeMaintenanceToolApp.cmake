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

if(CERT STREQUAL "-")
    message(STATUS "MakeMaintenanceToolApp: creating maintenance tool (ad-hoc)")
    execute_process(
        COMMAND "${BINARYCREATOR}" --config "${CONFIG}" --create-maintenancetool
        WORKING_DIRECTORY "${DATA_DIR}"
        RESULT_VARIABLE _rc)
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR "MakeMaintenanceToolApp: binarycreator failed (rc=${_rc})")
    endif()
    if(NOT EXISTS "${APP_OUT}")
        message(FATAL_ERROR "MakeMaintenanceToolApp: expected ${APP_OUT} was not produced")
    endif()
    execute_process(COMMAND codesign --force --deep --sign - "${APP_OUT}" RESULT_VARIABLE _rc)
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR "MakeMaintenanceToolApp: ad-hoc codesign failed (rc=${_rc})")
    endif()
else()
    message(STATUS "MakeMaintenanceToolApp: creating + signing maintenance tool with '${CERT}'")
    execute_process(
        COMMAND "${BINARYCREATOR}" --config "${CONFIG}" --create-maintenancetool --sign "${CERT}"
        WORKING_DIRECTORY "${DATA_DIR}"
        RESULT_VARIABLE _rc)
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR "MakeMaintenanceToolApp: binarycreator --create-maintenancetool failed (rc=${_rc})")
    endif()
    if(NOT EXISTS "${APP_OUT}")
        message(FATAL_ERROR "MakeMaintenanceToolApp: expected ${APP_OUT} was not produced")
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
        RESULT_VARIABLE _rc)
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR "MakeMaintenanceToolApp: notarytool submit failed (rc=${_rc})")
    endif()
    execute_process(COMMAND xcrun stapler staple "${APP_OUT}" RESULT_VARIABLE _rc)
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR "MakeMaintenanceToolApp: stapler staple failed (rc=${_rc})")
    endif()
    file(REMOVE "${_zip}")
endif()

message(STATUS "MakeMaintenanceToolApp: built ${APP_OUT}")
