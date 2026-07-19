if(NOT APP OR NOT NOTARY_PROFILE)
    message(FATAL_ERROR "NotarizeMacApp: need -DAPP= and -DNOTARY_PROFILE=")
endif()

if(NOT EXISTS "${APP}")
    message(FATAL_ERROR "NotarizeMacApp: app not found at ${APP}")
endif()

set(_zip "${APP}.notarize.zip")
file(REMOVE "${_zip}")
execute_process(
    COMMAND ditto -c -k --keepParent "${APP}" "${_zip}"
    RESULT_VARIABLE _rc)
if(NOT _rc EQUAL 0)
    message(FATAL_ERROR "NotarizeMacApp: ditto zip failed (rc=${_rc})")
endif()

message(STATUS "NotarizeMacApp: notarizing ${_zip}")
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
        message(STATUS "NotarizeMacApp: fetching notarization log for ${_sub_id}")
        execute_process(
            COMMAND xcrun notarytool log "${_sub_id}"
                    --keychain-profile "${NOTARY_PROFILE}"
            OUTPUT_VARIABLE _log ERROR_VARIABLE _logerr)
        message(STATUS "notarytool log:\n${_log}${_logerr}")
    endif()
    message(FATAL_ERROR "NotarizeMacApp: notarization did not succeed (see log above)")
endif()

execute_process(COMMAND xcrun stapler staple "${APP}" RESULT_VARIABLE _rc)
if(NOT _rc EQUAL 0)
    message(FATAL_ERROR "NotarizeMacApp: stapler staple failed (rc=${_rc})")
endif()
file(REMOVE "${_zip}")

message(STATUS "NotarizeMacApp: notarized + stapled ${APP}")
