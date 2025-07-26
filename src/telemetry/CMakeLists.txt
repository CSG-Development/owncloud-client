set(LIB_NAME telemetry)

if (WITH_CAP_CORE)

    set(CAPCORE_SRC
        ${CAPCORE_DIR}/ConfigCheckUtility.cpp
        ${CAPCORE_DIR}/Message.cpp
        ${CAPCORE_DIR}/MessageWrapper.cpp
        ${CAPCORE_DIR}/TelemetryClient.cpp
        ${CAPCORE_DIR}/TelemetryUtils.cpp
        ${CAPCORE_DIR}/TelemetryWrapper.cpp
        ${CAPCORE_DIR}/external/jsoncpp-amalgamated/jsoncpp.cpp
        ${CAPCORE_DIR}/Processor/AsCapableProcessor.cpp
        ${CAPCORE_DIR}/Processor/AsReceivedProcessor.cpp
        ${CAPCORE_DIR}/Processor/DeleteFailedEventTask.cpp
        ${CAPCORE_DIR}/Processor/PeriodicBatchedProcessor.cpp
        ${CAPCORE_DIR}/Processor/Processor.cpp
        ${CAPCORE_DIR}/Processor/VolumeBatchedProcessor.cpp
        ${CAPCORE_DIR}/platforms/x64/TelemetryUtils_win.cpp
        ${CAPCORE_DIR}/module/ModuleWrapper.cpp
        ${CAPCORE_DIR}/module/win/WinModuleWrapper.cpp
        ${CAPCORE_DIR}/db/DBUtils.cpp
        ${CAPCORE_DIR}/db/SqliteDatabase.cpp
        ${CAPCORE_DIR}/db/TelemetryClientDatabase.cpp
        ${CAPCORE_DIR}/external/SQLiteCpp/src/Backup.cpp
        ${CAPCORE_DIR}/external/SQLiteCpp/src/Column.cpp
        ${CAPCORE_DIR}/external/SQLiteCpp/src/Database.cpp
        ${CAPCORE_DIR}/external/SQLiteCpp/src/Exception.cpp
        ${CAPCORE_DIR}/external/SQLiteCpp/src/Savepoint.cpp
        ${CAPCORE_DIR}/external/SQLiteCpp/src/Statement.cpp
        ${CAPCORE_DIR}/external/SQLiteCpp/src/Transaction.cpp
        ${CAPCORE_DIR}/external/SQLiteCpp/sqlite3/sqlite3.c
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
)
