set(LIBRARY_NAME customui)

add_library(${LIBRARY_NAME} STATIC
    style_tool_button_menu.cpp
    style_tool_button_menu.h
    arrow_toolbutton.cpp
    arrow_toolbutton.h
    dimwidget.cpp
    dimwidget.h
    combowidget.cpp
    combowidget.h
    combowidget.ui
    menu_toolbutton.cpp
    menu_toolbutton.h
    tool_button_dots.cpp
    tool_button_dots.h
    style_push_button.cpp
    style_push_button.h
    stylebase.cpp
    stylebase.h
    stylehelper.cpp
    stylehelper.h
    ccheckbox.cpp
    ccheckbox.h
    checkboxres.cpp
    checkboxres.h
    radioindicatorproxy.h
    inputwidget.cpp
    inputwidget.h
    inputwidget.ui
    popupcombomodel.cpp
    popupcombomodel.h
    popupcombowidget.cpp
    popupcombowidget.h
    popupcombowidget.ui
    progressindicator.cpp
    progressindicator.h
    overlaywidget.cpp
    overlaywidget.h
    loginpushbutton.cpp
    loginpushbutton.h
    focusproxy.h
    customui_res.qrc
    codeinputwidget.cpp
    codeinputwidget.h
    codeinputwidget.ui
    tooltip_manager.cpp
    tooltip_manager.h
    custom_tooltip.cpp
    custom_tooltip.h
    toastwidget.cpp
    toastwidget.h
    toastwidget.ui
)

target_link_libraries(${LIBRARY_NAME} PUBLIC 
    Qt::Widgets
    # Qt::Svg
    PersonalCloudResources
    libsync
)

set_target_properties(${LIBRARY_NAME} PROPERTIES AUTOUIC ON AUTORCC ON)
target_include_directories(${LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

apply_common_target_settings(${LIBRARY_NAME})
