set(LIBRARY_NAME remoteaccess)

add_library(${LIBRARY_NAME} STATIC
    codedialog.cpp
    codedialog.h
    codedialog.ui
    devwidget.cpp
    devwidget.h
    devwidget.ui
    errordialog.cpp
    errordialog.h
    errordialog.ui
    overlaycontroller.cpp
    overlaycontroller.h
)

target_link_libraries(${LIBRARY_NAME} PUBLIC
    Qt::Widgets
    Qt::Concurrent
    libsync
    PersonalCloudResources
)

set_target_properties(${LIBRARY_NAME} PROPERTIES AUTOUIC ON AUTORCC ON)
target_include_directories(${LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

app_apply_common_target_settings(${LIBRARY_NAME})
