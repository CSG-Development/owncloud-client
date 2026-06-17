set(LIB_NAME telemetry)

if (WITH_CAP_CORE)

    set(CAPCORE_SRC
        ProviderCapcore.cpp
        ProviderCapcore.h
    )

endif(WITH_CAP_CORE)

add_library(${LIB_NAME} STATIC
    ProviderBase.h
    ProviderNull.h
    Telemetry.cpp
    Telemetry.h
    ${CAPCORE_SRC}
)

target_link_libraries(${LIB_NAME} PUBLIC
    Qt::Core
    ${CURL_LIB}
    ${CAPCORE_LIB}
)

if(WITH_CAP_CORE)
    target_include_directories(${LIB_NAME} PRIVATE ${CAPCORE_INCLUDE_DIRS})
    target_compile_definitions(${LIB_NAME} PRIVATE ${CAPCORE_COMPILE_DEFINITIONS})
    target_compile_options(${LIB_NAME} PRIVATE ${CAPCORE_COMPILE_OPTIONS})
    target_link_directories(${LIB_NAME} PUBLIC ${CAPCORE_LINK_DIRECTORIES})
endif()
