find_package(KDSingleApplication-qt6 1.0.0 REQUIRED)

set(client_SRCS
    aboutdialog.cpp
    aboutdialog.h
    aboutdialog.ui
    accountmanager.cpp
    accountmanager.h
    accountsettings.cpp
    accountsettings.h
    accountsettings.ui
    activitydata.cpp
    activitydata.h
    activitywidget.cpp
    activitywidget.h
    activitywidget.ui
    accountstate.cpp
    accountstate.h
    application.cpp
    application.h
    applicationgui.cpp
    applicationgui.h
    askexperimentalvirtualfilesfeaturemessagebox.cpp
    askexperimentalvirtualfilesfeaturemessagebox.h
    clientproxy.cpp
    clientproxy.h
    commonstrings.cpp
    commonstrings.h
    connectionvalidator.cpp
    connectionvalidator.h
    elidedlabel.cpp
    elidedlabel.h
    fetchserversettings.cpp
    fetchserversettings.h
    folder.cpp
    folder.h
    folderdefinition.cpp
    folderdefinition.h
    folderman.cpp
    folderman.h
    folderstatusmodel.cpp
    folderstatusmodel.h
    folderstatusdelegate.cpp
    folderstatusdelegate.h
    foldertreeview.cpp
    foldertreeview.h
    folderwatcher.cpp
    folderwatcher.h
    generalsettings.cpp
    generalsettings.h
    generalsettings.ui
    guiutility.cpp
    guiutility.h
    ignorelisteditor.cpp
    ignorelisteditor.h
    ignorelisteditor.ui
    issueswidget.cpp
    issueswidget.h
    issueswidget.ui
    lockwatcher.cpp
    lockwatcher.h
    networksettings.cpp
    networksettings.h
    networksettings.ui
    notificationwidget.cpp
    notificationwidget.h
    notificationwidget.ui
    notificationconfirmjob.cpp
    notificationconfirmjob.h
    ocssharejob.cpp
    ocssharejob.h
    openfilemanager.cpp
    openfilemanager.h
    protocolwidget.cpp
    protocolwidget.h
    protocolwidget.ui
    protocolitem.cpp
    protocolitem.h
    quotainfo.cpp
    quotainfo.h
    selectivesyncwidget.cpp
    selectivesyncwidget.h
    settingsdialog.cpp
    settingsdialog.h
    settingsdialog.ui
    servernotificationhandler.cpp
    sharedialog.cpp
    sharedialog.h
    sharedialog.ui
    sharelinkwidget.cpp
    sharelinkwidget.h
    sharelinkwidget.ui
    sharemanager.cpp
    sharemanager.h
    shareusergroupwidget.cpp
    shareusergroupwidget.h
    shareusergroupwidget.ui
    shareuserline.ui
    sharee.cpp
    sharee.h
    spacemigration.cpp
    spacemigration.h
    syncrunfilelog.cpp
    syncrunfilelog.h
    systray.cpp
    systray.h
    thumbnailjob.cpp
    thumbnailjob.h
    tlserrordialog.cpp
    tlserrordialog.h
    tlserrordialog.ui
    tooltipupdater.cpp
    tooltipupdater.h
    translations.cpp
    translations.h
    updateurldialog.cpp
    updateurldialog.h

    creds/httpcredentialsgui.cpp
    creds/httpcredentialsgui.h
    models/activitylistmodel.cpp
    models/activitylistmodel.h
    models/expandingheaderview.cpp
    models/expandingheaderview.h
    models/models.cpp
    models/models.h
    models/protocolitemmodel.cpp
    models/protocolitemmodel.h
    scheduling/syncscheduler.cpp
    scheduling/syncscheduler.h
    scheduling/etagwatcher.cpp
    scheduling/etagwatcher.h
)

set(3rdparty_SRC
    ../3rdparty/QProgressIndicator/QProgressIndicator.cpp
   )

set( final_src
    ${MIRALL_RC_SRC}
    ${client_SRCS}
    ${3rdparty_SRC}
)

add_subdirectory(newwizard)
add_subdirectory(folderwizard)
add_subdirectory(loginrequireddialog)
add_subdirectory(customui)
add_subdirectory(customdialogs)
add_subdirectory(remoteaccess)

add_library(PersonalCloudCore STATIC ${final_src})
set_target_properties(PersonalCloudCore PROPERTIES AUTOUIC ON AUTORCC ON)
target_link_libraries(PersonalCloudCore PUBLIC
    Qt::Widgets 
    Qt::Network 
    Qt::Xml 
    Qt::Concurrent
    newwizard
    folderwizard 
    spaces 
    loginrequireddialog
    customui
    customdialogs
    remoteaccess
    libsync
#   telemetry
    Qt6Keychain::Qt6Keychain
)

app_apply_common_target_settings(PersonalCloudCore)

add_subdirectory(spaces)
add_subdirectory(socketapi)

target_include_directories(PersonalCloudCore PUBLIC
    ${CMAKE_SOURCE_DIR}/src/3rdparty/QProgressIndicator
    ${CMAKE_SOURCE_DIR}/src/3rdparty/qtlockedfile
    ${CMAKE_SOURCE_DIR}/src/3rdparty/qtsingleapplication
    ${CMAKE_CURRENT_BINARY_DIR}
)

IF( APPLE )
    target_sources(PersonalCloudCore PRIVATE
        systray.mm
        settingsdialog_mac.mm
        guiutility_mac.mm
        folderwatcher_mac.cpp)
    set_source_files_properties(guiutility_mac.mm PROPERTIES COMPILE_DEFINITIONS SOCKETAPI_TEAM_IDENTIFIER_PREFIX="${SOCKETAPI_TEAM_IDENTIFIER_PREFIX}")
elseif( WIN32 )
    target_sources(PersonalCloudCore PRIVATE
        guiutility_win.cpp
        folderwatcher_win.cpp)
endif()

if(WITH_AUTO_UPDATER)
    add_subdirectory(updater)
    target_compile_definitions(PersonalCloudCore PRIVATE WITH_AUTO_UPDATER)
endif()

add_executable(PersonalCloud main.cpp)
set_target_properties(PersonalCloud PROPERTIES
    OUTPUT_NAME "${APPLICATION_EXECUTABLE}"
    AUTOUIC ON
    AUTORCC ON
)
app_apply_common_target_settings(PersonalCloud)
target_link_libraries(PersonalCloud
    PersonalCloudCore
    PersonalCloudResources
    KDAB::kdsingleapplication
    telemetry
)

MESSAGE(STATUS "APPLICATION_SIDEBAR_ICONS: ${APPLICATION_ICON_NAME}: ${APPLICATION_SIDEBAR_ICONS}")

add_application_icon(appIcons)
target_sources(PersonalCloud PRIVATE ${appIcons})

if(WIN32)
    target_sources(PersonalCloud PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/PersonalCloud.exe.manifest)

    set_target_properties(PersonalCloud PROPERTIES WIN32_EXECUTABLE TRUE)

    # we may not add MACOSX_BUNDLE here, if not building one
else()
    target_sources(PersonalCloud PRIVATE ${APPLICATION_BUNDLED_RESOURCES})

    set_source_files_properties(
      ${APPLICATION_BUNDLED_RESOURCES}
      PROPERTIES
      MACOSX_PACKAGE_LOCATION Resources
      )

  # Build a real .app bundle on macOS so install/deploy/IFW packaging produce
  # PersonalCloud.app (the MACOSX_BUNDLE_* properties below are only honored when
  # MACOSX_BUNDLE is TRUE). app_install_target installs it via BUNDLE DESTINATION ".".
  set_target_properties(PersonalCloud PROPERTIES
      MACOSX_BUNDLE TRUE
      MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/MacOSXBundleInfo.plist)

  if(APPLE)
      set_target_properties(PersonalCloud PROPERTIES
          INSTALL_RPATH "@executable_path/../Frameworks")
  endif()
endif()

app_install_target(PersonalCloud)
app_deploy_runtime(PersonalCloud)
