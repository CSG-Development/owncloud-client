if (DEFINED CAPCORE_DIR)
    message(STATUS "Using CapCore at ${CAPCORE_DIR}")
else()
    message(FATAL_ERROR "CAPCORE_DIR is not defined")
endif()

set(CAPCORE_INCLUDE_DIRS
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

set(CAPCORE_COMPILE_DEFINITIONS USE_CAPCORE)
set(CAPCORE_COMPILE_OPTIONS)
set(CAPCORE_LINK_DIRECTORIES)

if (APPLE)

    list(APPEND CAPCORE_COMPILE_OPTIONS -fexceptions)
    list(APPEND CAPCORE_LINK_DIRECTORIES "${CAPCORE_DIR}/lib")
    set(CURL_LIB curl)
    set(CAPCORE_LIB telemetry_core_staticlib)
    # Ensure static libraries linked
    set(CURL_LIB
        libcurl.a
        libbrotlicommon.a
        libbrotlidec.a
        libbrotlienc.a
        libnghttp2.a
        libnghttp3.a
        libssh2.a
        libzstd.a
        libcrypto.a
        libssl.a
        libidn2.a
        "-framework SystemConfiguration"
    )

elseif (WIN32)

    list(APPEND CAPCORE_COMPILE_DEFINITIONS CURL_STATICLIB)
    list(APPEND CAPCORE_LINK_DIRECTORIES ${CAPCORE_DIR}/platforms/x64/prebuilt/lib)
    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        set(CURL_LIB libcurl-d)
        set(CAPCORE_LIB telemetry_cored)
    else()
        set(CURL_LIB libcurl)
        set(CAPCORE_LIB telemetry_core)
    endif()

endif()
