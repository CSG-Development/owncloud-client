find_package(LibreGraphAPI 1.0.4 REQUIRED)
find_package(OpenSSL REQUIRED)

set(TARGET libsync)

set_package_properties(LibreGraphAPI PROPERTIES
        URL https://github.com/owncloud/libre-graph-api-cpp-qt-client.git
        DESCRIPTION "Libre Graph is a free API for cloud collaboration inspired by the MS Graph API."
        TYPE OPTIONAL
)

set(SOURCES
    account.cpp
    bandwidthmanager.cpp
    capabilities.cpp
    cookiejar.cpp
    discovery.cpp
    discoveryphase.cpp
    filesystem.cpp
    httplogger.cpp
    jobqueue.cpp
    logger.cpp
    accessmanager.cpp
    configfile.cpp
    abstractnetworkjob.cpp
    networkjobs.cpp
    personalcloudpropagator.cpp
    applicationtheme.cpp
    platform.cpp
    progressdispatcher.cpp
    propagatorjobs.cpp
    propagatedownload.cpp
    propagateupload.cpp
    propagateuploadv1.cpp
    propagateuploadng.cpp
    propagateuploadtus.cpp
    propagateremotedelete.cpp
    propagateremotemove.cpp
    propagateremotemkdir.cpp
    syncengine.cpp
    endpointrecoveryevent.h
    syncendpointrecovery.cpp
    syncendpointrecovery.h
    syncfileitem.cpp
    syncfilestatustracker.cpp
    localdiscoverytracker.cpp
    syncresult.cpp
    syncoptions.cpp
    theme.cpp
    creds/credentialmanager.cpp
    creds/proxycredentials.cpp
    creds/abstractcredentials.cpp
    creds/oauth.cpp
    creds/webfinger.cpp
    creds/httpcredentials.cpp

    networkjobs/fetchuserinfojobfactory.cpp
    networkjobs/checkserverjobfactory.cpp
    networkjobs/jsonjob.cpp
    networkjobs/resources.cpp
    networkjobs/simpleresolveurljobfactory.cpp
    networkjobs/simpleresolveurljobfactory.h

    device/networkmonitor.cpp
    device/networkmonitor.h
    device/apiclient.cpp
    device/apiclient.h
    device/mdnsclient.cpp
    device/mdnsclient.h
    device/devicetypes.cpp
    device/devicetypes.h
    device/deviceapi.cpp
    device/deviceapi.h
    device/devicedefines.cpp
    device/devicedefines.h
    device/deviceaggregator.cpp
    device/deviceaggregator.h
    device/devicepathresolver.cpp
    device/devicepathresolver.h
    device/devicecontroller.cpp
    device/devicecontroller.h

    abstractcorejob.cpp
    determineauthtypejobfactory.cpp

    appprovider.cpp

    certificates/certificatevalidator.cpp
    certificates/certificatevalidator.h
    certs.qrc
)

if(WIN32)
    list(APPEND SOURCES platform_win.cpp)
elseif(APPLE)
    list(APPEND SOURCES platform_mac.mm platform_mac_deprecated.mm)
endif()

# These headers are installed for libpersonalcloudsync to be used by 3rd party apps
INSTALL(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/personalcloudlib.h
        logger.h
        accessmanager.h
    DESTINATION ${APP_INSTALL_INCLUDEDIR}/libsync
)

add_library(${TARGET} SHARED ${SOURCES})
set_target_properties(${TARGET} PROPERTIES EXPORT_NAME SyncCore)

target_link_libraries(${TARGET}
    PUBLIC
        csync
        Qt::Core
        Qt::Network
        Qt::Widgets
        OpenSSL::SSL
        OpenSSL::Crypto
    PRIVATE
        PersonalCloudResources
        Qt::Concurrent
        ZLIB::ZLIB
        Qt6Keychain::Qt6Keychain
)

app_apply_common_target_settings(${TARGET})

set_source_files_properties(configfile.cpp
    PROPERTIES
        COMPILE_DEFINITIONS EXCLUDE_FILE_NAME="${EXCLUDE_FILE_NAME}"
)

target_link_libraries(${TARGET} PUBLIC $<BUILD_INTERFACE:OpenAPI::LibreGraphAPI>)
add_subdirectory(graphapi)

if ( APPLE )
    target_link_libraries(${TARGET} PUBLIC
         /System/Library/Frameworks/CoreServices.framework
         /System/Library/Frameworks/Foundation.framework
         /System/Library/Frameworks/AppKit.framework
         /System/Library/Frameworks/IOKit.framework
    )
endif()

if(NO_MSG_HANDLER)
    target_compile_definitions(${TARGET} PRIVATE -DNO_MSG_HANDLER=1)
endif()

GENERATE_EXPORT_HEADER(${TARGET}
        EXPORT_MACRO_NAME APPLICATIONSYNC_EXPORT
        EXPORT_FILE_NAME personalcloudlib.h
        STATIC_DEFINE APPLICATION_BUILT_AS_STATIC
)

set_target_properties(${TARGET} PROPERTIES
        OUTPUT_NAME "${synclib_NAME}"
        VERSION ${MIRALL_VERSION}
        SOVERSION ${MIRALL_SOVERSION}
)

if(WITH_CRASHREPORTER)
    set_source_files_properties(platform.cpp PROPERTIES COMPILE_DEFINITIONS CRASHREPORTER_EXECUTABLE="${CRASHREPORTER_EXECUTABLE}")
    target_link_libraries(${TARGET} PRIVATE CrashReporterQt::Handler)
endif()


app_install_target(${TARGET} EXPORT ${APPLICATION_SHORTNAME}Config)
