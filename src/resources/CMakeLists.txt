add_library(curatorResources SHARED
    client.qrc
    core_theme.qrc
    loadresources.cpp
    resources.cpp
)

generate_theme(curatorResources CURATOR_SIDEBAR_ICONS)

# make them available to the whole project
set(CURATOR_SIDEBAR_ICONS ${CURATOR_SIDEBAR_ICONS} CACHE INTERNAL "Sidebar icons" FORCE)

target_link_libraries(curatorResources PUBLIC Qt::Core Qt::Gui)
apply_common_target_settings(curatorResources)
target_include_directories(curatorResources PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..> $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>)

set_target_properties(curatorResources PROPERTIES
    OUTPUT_NAME "${APPLICATION_LIB_PREFIX}Resources"
    AUTOUIC ON
    AUTORCC ON
    VERSION ${MIRALL_VERSION}
    SOVERSION ${MIRALL_SOVERSION}
)

generate_export_header(curatorResources
        EXPORT_MACRO_NAME CURATORRESOURCES_EXPORT
        EXPORT_FILE_NAME curatorresources.h
        STATIC_DEFINE CURATOR_BUILT_AS_STATIC
)

# Handle Translations, pick all client_* files from trans directory.
file(GLOB client_translations ${CMAKE_SOURCE_DIR}/translations/client_*.ts)
qt_add_lrelease(curatorResources TS_FILES ${client_translations} QM_FILES_OUTPUT_VARIABLE client_compiled_translations)
add_resources_to_target(
    TARGET curatorResources
    PREFIX translations
    FILES "${client_compiled_translations}"
)

install(TARGETS curatorResources EXPORT ${APPLICATION_SHORTNAME}Config ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
