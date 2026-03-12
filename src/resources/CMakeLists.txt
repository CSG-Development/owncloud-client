add_library(PersonalCloudResources SHARED
    client.qrc
    core_theme.qrc
    loadresources.cpp
    resources.cpp
)

generate_theme(PersonalCloudResources APPLICATION_SIDEBAR_ICONS)

# make them available to the whole project
set(APPLICATION_SIDEBAR_ICONS ${APPLICATION_SIDEBAR_ICONS} CACHE INTERNAL "Sidebar icons" FORCE)

target_link_libraries(PersonalCloudResources PUBLIC Qt::Core Qt::Gui)
apply_common_target_settings(PersonalCloudResources)
target_include_directories(PersonalCloudResources PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..> $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>)

set_target_properties(PersonalCloudResources PROPERTIES
    OUTPUT_NAME "${APPLICATION_LIB_PREFIX}Resources"
    AUTOUIC ON
    AUTORCC ON
    VERSION ${MIRALL_VERSION}
    SOVERSION ${MIRALL_SOVERSION}
)

generate_export_header(PersonalCloudResources
        EXPORT_MACRO_NAME APPLICATIONRESOURCES_EXPORT
        EXPORT_FILE_NAME personalcloudresources.h
        STATIC_DEFINE APPLICATION_BUILT_AS_STATIC
)

# Handle Translations, pick all client_* files from trans directory.
file(GLOB client_translations ${CMAKE_SOURCE_DIR}/translations/client_*.ts)
qt_add_lrelease(PersonalCloudResources TS_FILES ${client_translations} QM_FILES_OUTPUT_VARIABLE client_compiled_translations)
add_resources_to_target(
    TARGET PersonalCloudResources
    PREFIX translations
    FILES "${client_compiled_translations}"
)

install(TARGETS PersonalCloudResources EXPORT ${APPLICATION_SHORTNAME}Config ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
