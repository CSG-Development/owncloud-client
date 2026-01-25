add_library(loginrequireddialog STATIC
    loginrequireddialog.cpp
    loginrequireddialog.ui

    abstractloginwidget.cpp

    oauthloginwidget.cpp
    oauthloginwidget.ui

    basicloginwidget.cpp
    basicloginwidget.ui
)

if(APPLE)
    string(REPLACE "-fno-exceptions" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    add_compile_options(-fexceptions)
endif()

target_link_libraries(loginrequireddialog PUBLIC Qt::Widgets libsync curatorResources)
set_target_properties(loginrequireddialog PROPERTIES AUTOUIC ON AUTORCC ON)
target_include_directories(loginrequireddialog PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
apply_common_target_settings(loginrequireddialog)
