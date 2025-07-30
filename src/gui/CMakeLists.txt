include(ECMAddAppIcon)

find_package(KDSingleApplication-qt6 1.0.0 REQUIRED)

set(client_UI_SRCS
    aboutdialog.ui
    accountsettings.ui
    generalsettings.ui
    ignorelisteditor.ui
    networksettings.ui
    protocolwidget.ui
    issueswidget.ui
    activitywidget.ui
    settingsdialog.ui
    sharedialog.ui
    sharelinkwidget.ui
    shareusergroupwidget.ui
    shareuserline.ui
    tlserrordialog.ui
    notificationwidget.ui
    logbrowser.ui
)

set(client_SRCS
    aboutdialog.cpp
    accountmanager.cpp
    accountsettings.cpp
    application.cpp
    askexperimentalvirtualfilesfeaturemessagebox.cpp
    fetchserversettings.cpp
    clientproxy.cpp
    commonstrings.cpp
    connectionvalidator.cpp
    folder.cpp
    folderman.cpp
    folderstatusmodel.cpp
    folderstatusdelegate.cpp
    folderwatcher.cpp
    generalsettings.cpp
    ignorelisteditor.cpp
    lockwatcher.cpp
    logbrowser.cpp
    networksettings.cpp
    ocssharejob.cpp
    openfilemanager.cpp
    curatorgui.cpp
    protocolwidget.cpp
    protocolitem.cpp
    issueswidget.cpp
    activitydata.cpp
    activitywidget.cpp
    selectivesyncwidget.cpp
    settingsdialog.cpp
    sharedialog.cpp
    sharelinkwidget.cpp
    sharemanager.cpp
    shareusergroupwidget.cpp
    sharee.cpp
    tlserrordialog.cpp
    syncrunfilelog.cpp
    systray.cpp
    thumbnailjob.cpp
    quotainfo.cpp
    accountstate.cpp
    tooltipupdater.cpp
    notificationwidget.cpp
    notificationconfirmjob.cpp
    servernotificationhandler.cpp
    guiutility.cpp
    elidedlabel.cpp
    translations.cpp
    creds/httpcredentialsgui.cpp
    updateurldialog.cpp

    models/activitylistmodel.cpp
    models/expandingheaderview.cpp
    models/models.cpp
    models/protocolitemmodel.cpp

    spacemigration.cpp

    scheduling/syncscheduler.cpp
    scheduling/etagwatcher.cpp
)

set(3rdparty_SRC
    ../3rdparty/QProgressIndicator/QProgressIndicator.cpp
   )

set( final_src
    ${MIRALL_RC_SRC}
    ${client_SRCS}
    ${client_UI_SRCS}
    ${3rdparty_SRC}
)

add_subdirectory(newwizard)
add_subdirectory(folderwizard)
add_subdirectory(loginrequireddialog)

add_library(curatorCore STATIC ${final_src})
set_target_properties(curatorCore PROPERTIES AUTOUIC ON AUTORCC ON)
target_link_libraries(curatorCore
    PUBLIC
        Qt::Widgets Qt::Network Qt::Xml
        newwizard folderwizard spaces loginrequireddialog
        libsync
#        telemetry
        Qt6Keychain::Qt6Keychain
)

apply_common_target_settings(curatorCore)


add_subdirectory(spaces)


add_subdirectory(socketapi)

target_include_directories(curatorCore PUBLIC
    ${CMAKE_SOURCE_DIR}/src/3rdparty/QProgressIndicator
    ${CMAKE_SOURCE_DIR}/src/3rdparty/qtlockedfile
    ${CMAKE_SOURCE_DIR}/src/3rdparty/qtsingleapplication
    ${CMAKE_CURRENT_BINARY_DIR}
)

IF( APPLE )
    target_sources(curatorCore PRIVATE
            systray.mm
            settingsdialog_mac.mm
            guiutility_mac.mm
            folderwatcher_mac.cpp)
    set_source_files_properties(guiutility_mac.mm PROPERTIES COMPILE_DEFINITIONS SOCKETAPI_TEAM_IDENTIFIER_PREFIX="${SOCKETAPI_TEAM_IDENTIFIER_PREFIX}")
elseif( WIN32 )
    target_sources(curatorCore PRIVATE
            guiutility_win.cpp
            folderwatcher_win.cpp)
elseif(UNIX AND NOT APPLE)
    ## handle DBUS for Fdo notifications
    if (TARGET Qt::DBus)
        target_link_libraries(curatorCore PUBLIC Qt::DBus)
        target_compile_definitions(curatorCore PUBLIC "USE_FDO_NOTIFICATIONS")
    endif()
    target_sources(curatorCore PRIVATE
            folderwatcher_linux.cpp
            guiutility_unix.cpp)
endif()

if(WITH_AUTO_UPDATER)
    add_subdirectory(updater)
    target_compile_definitions(curatorCore PRIVATE WITH_AUTO_UPDATER)
endif()

add_executable(curator main.cpp)
set_target_properties(curator PROPERTIES
    OUTPUT_NAME "${APPLICATION_EXECUTABLE}"
    AUTOUIC ON
    AUTORCC ON
)
apply_common_target_settings(curator)
target_link_libraries(curator
    curatorCore
    curatorResources
    KDAB::kdsingleapplication
    telemetry
)

MESSAGE(STATUS "CURATOR_SIDEBAR_ICONS: ${APPLICATION_ICON_NAME}: ${CURATOR_SIDEBAR_ICONS}")

ecm_add_app_icon(appIcons ICONS "${CURATOR_ICONS}" SIDEBAR_ICONS "${CURATOR_SIDEBAR_ICONS}" OUTFILE_BASENAME "${APPLICATION_ICON_NAME}")
target_sources(curator PRIVATE ${appIcons})

if(NOT APPLE)
    if(WIN32)
        target_sources(curator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/curator.exe.manifest)
    else()
        file(GLOB _icons "${OEM_THEME_DIR}/theme/colored/*-${APPLICATION_ICON_NAME}-icon.png")
        foreach(_file ${_icons})
            string(REPLACE "${OEM_THEME_DIR}/theme/colored/" "" _res ${_file})
            string(REPLACE "-${APPLICATION_ICON_NAME}-icon.png" "" _res ${_res})
            install(FILES ${_file} RENAME ${APPLICATION_ICON_NAME}.png DESTINATION ${KDE_INSTALL_DATADIR}/icons/hicolor/${_res}x${_res}/apps)
        endforeach(_file)
    endif()

    # we may not add MACOSX_BUNDLE here, if not building one
else()
    target_sources(curator PRIVATE ${CURATOR_BUNDLED_RESOURCES})

    set_source_files_properties(
      ${CURATOR_BUNDLED_RESOURCES}
      PROPERTIES
      MACOSX_PACKAGE_LOCATION Resources
      )

  set_target_properties(curator PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/MacOSXBundleInfo.plist)
endif()

install(TARGETS curator ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

if(UNIX AND NOT APPLE)
    configure_file(${CMAKE_SOURCE_DIR}/curator.desktop.in
                   ${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_EXECUTABLE}.desktop)
    install(FILES  ${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_EXECUTABLE}.desktop DESTINATION ${KDE_INSTALL_DATADIR}/applications )

    configure_file(curator.xml.in ${APPLICATION_EXECUTABLE}.xml)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_EXECUTABLE}.xml DESTINATION ${KDE_INSTALL_DATADIR}/mime/packages )

    find_package(SharedMimeInfo)
    if(SharedMimeInfo_FOUND)
        update_xdg_mimetypes( ${KDE_INSTALL_DATADIR}/mime/packages )
    endif(SharedMimeInfo_FOUND)
endif()
