set(LIBRARY_NAME customui)

add_library(${LIBRARY_NAME} STATIC
    style_tool_button_menu.cpp
    style_tool_button_menu.h
    arrow_toolbutton.cpp
    arrow_toolbutton.h
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
    cradiobutton.cpp
    cradiobutton.h
    radiobuttonres.cpp
    radiobuttonres.h
    inputwidget.cpp
    inputwidget.h
    progressindicator.cpp
    progressindicator.h
    loginpushbutton.cpp
    loginpushbutton.h
    focusproxy.cpp
    focusproxy.h
    customui_res.qrc
)

target_link_libraries(${LIBRARY_NAME} PUBLIC 
	Qt::Widgets 
	curatorResources
    libsync
)

set_target_properties(${LIBRARY_NAME} PROPERTIES AUTOUIC ON AUTORCC ON)
target_include_directories(${LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

apply_common_target_settings(${LIBRARY_NAME})
