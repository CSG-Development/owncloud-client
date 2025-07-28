if (DEFINED CAPCORE_DIR)
    message(STATUS "Using CapCore at ${CAPCORE_DIR}")
else()
    message(FATAL_ERROR "CAPCORE_DIR is not defined")
endif()

include_directories(
    ${CAPCORE_DIR}
    ${CAPCORE_DIR}/external/jsoncpp-amalgamated
    ${CAPCORE_DIR}/external/curl/include
    ${CAPCORE_DIR}/external/SQLiteCpp/include
    ${CAPCORE_DIR}/external/SQLiteCpp/sqlite3
    ${CAPCORE_DIR}/Processor
    ${CAPCORE_DIR}/model
    ${CAPCORE_DIR}/module
    ${CAPCORE_DIR}/db
)

add_definitions(-DUSE_CAPCORE)
add_definitions(-DCURL_STATICLIB)

link_directories(
    ${CAPCORE_DIR}/platforms/x64/prebuilt/lib
)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message(STATUS "CapCore Debug build")
    set(CURL_LIB libcurl-d)
    set(CAPCORE_LIB telemetry_cored)
else()
    message(STATUS "CapCore Release build")
    set(CURL_LIB libcurl)
    set(CAPCORE_LIB telemetry_core)
endif()

