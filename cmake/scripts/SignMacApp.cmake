if(NOT APP OR NOT CERT)
    message(FATAL_ERROR "SignMacApp: APP and CERT are required")
endif()

set(_cs codesign --force --options runtime --timestamp --sign "${CERT}")

execute_process(
    COMMAND find "${APP}" -type f -perm -111 -exec ${_cs} {} +
    RESULT_VARIABLE _rc)
if(NOT _rc EQUAL 0)
    message(FATAL_ERROR "SignMacApp: signing nested binaries failed (rc=${_rc})")
endif()

if(ENTITLEMENTS AND EXISTS "${ENTITLEMENTS}" AND EXISTS "${APP}/Contents/PlugIns")
    execute_process(
        COMMAND find "${APP}/Contents/PlugIns" -type d -name *.appex
                -exec codesign --force --options runtime --timestamp
                      --entitlements "${ENTITLEMENTS}" --sign "${CERT}" {} +
        RESULT_VARIABLE _rc)
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR "SignMacApp: signing .appex failed (rc=${_rc})")
    endif()
endif()

execute_process(COMMAND ${_cs} "${APP}" RESULT_VARIABLE _rc)
if(NOT _rc EQUAL 0)
    message(FATAL_ERROR "SignMacApp: signing bundle failed (rc=${_rc})")
endif()

message(STATUS "SignMacApp: signed ${APP}")
