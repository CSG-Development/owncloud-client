set(LIBRARY_NAME customdialogs)

add_library(${LIBRARY_NAME} STATIC
    dlgutils.h
    custommessagebox.cpp
    custommessagebox.h
    custominputdlg.cpp
    custominputdlg.h
    logbrowserdlg.cpp
    logbrowserdlg.h

    platform/common/confirmdlgcontroller.cpp
    platform/common/confirmdlgcontroller.h
    platform/common/inputdlgcontroller.cpp
    platform/common/inputdlgcontroller.h
    platform/common/windowdragger.cpp
    platform/common/windowdragger.h
    platform/common/commondialogwidgets.h
    platform/common/baseconfirmdlg.cpp
    platform/common/baseconfirmdlg.h
    platform/common/baseinputdlg.cpp
    platform/common/baseinputdlg.h
    platform/common/pushbuttonstyle.h
    platform/common/prophelper.h
    platform/common/focusframe.h

    platform/windows/confirmdlg.cpp
    platform/windows/confirmdlg.h
    platform/windows/confirmdlg.ui
    platform/windows/inputdlg.cpp
    platform/windows/inputdlg.h
    platform/windows/inputdlg.ui
    platform/windows/logbrowser.cpp
    platform/windows/logbrowser.h
    platform/windows/logbrowser.ui

    platform/macos/confirmdlgmac.cpp
    platform/macos/confirmdlgmac.h
    platform/macos/confirmdlgmac.ui
    platform/macos/confirmwidedlgmac.cpp
    platform/macos/confirmwidedlgmac.h
    platform/macos/confirmwidedlgmac.ui
    platform/macos/custompushbuttonmac.cpp
    platform/macos/custompushbuttonmac.h
    platform/macos/inputdlgmac.cpp
    platform/macos/inputdlgmac.h
    platform/macos/inputdlgmac.ui
    platform/macos/logbrowsermac.cpp
    platform/macos/logbrowsermac.h
    platform/macos/logbrowsermac.ui

    customdialogs_res.qrc
)

target_link_libraries(${LIBRARY_NAME} PUBLIC 
    Qt::Widgets
    #Qt::Svg
    PersonalCloudResources
    libsync
)

set_target_properties(${LIBRARY_NAME} PROPERTIES AUTOUIC ON AUTORCC ON)
target_include_directories(${LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

app_apply_common_target_settings(${LIBRARY_NAME})
