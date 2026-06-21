if(NOT APP OR NOT CERT)
    message(FATAL_ERROR "SignMacApp: need -DAPP= and -DCERT=")
endif()

set(_cs codesign --force --options runtime --timestamp --sign "${CERT}")

execute_process(COMMAND ${_cs} --deep "${APP}" RESULT_VARIABLE _rc)
if(NOT _rc EQUAL 0)
    message(FATAL_ERROR "SignMacApp: deep sign failed (rc=${_rc})")
endif()

if(ENTITLEMENTS AND EXISTS "${ENTITLEMENTS}" AND EXISTS "${APP}/Contents/PlugIns")
    execute_process(
        COMMAND find "${APP}/Contents/PlugIns" -type d -name *.appex
                -exec ${_cs} --entitlements "${ENTITLEMENTS}" {} +
        RESULT_VARIABLE _rc)
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR "SignMacApp: appex sign failed (rc=${_rc})")
    endif()
    execute_process(COMMAND ${_cs} "${APP}" RESULT_VARIABLE _rc)
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR "SignMacApp: re-seal failed (rc=${_rc})")
    endif()
endif()

message(STATUS "SignMacApp: signed ${APP}")
