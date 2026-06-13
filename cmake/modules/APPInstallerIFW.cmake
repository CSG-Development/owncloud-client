# Qt Installer Framework packaging. Included only when BUILD_INSTALLER=ON.

set(QTIFW_VERSION "4.11" CACHE STRING "Qt Installer Framework version used for the automatic probe path")

# QTIFW_ROOT may point at the IFW dir or its bin/; otherwise probe next to Qt.
find_program(IFW_BINARYCREATOR
    NAMES binarycreator
    HINTS
        "${QTIFW_ROOT}/bin"
        "${QTIFW_ROOT}"
        "$ENV{QTIFW_ROOT}/bin"
        "$ENV{QTIFW_ROOT}"
        "${QT_ROOT}/../../Tools/QtInstallerFramework/${QTIFW_VERSION}/bin"
        "$ENV{QT_ROOT}/../../Tools/QtInstallerFramework/${QTIFW_VERSION}/bin"
    DOC "Qt Installer Framework binarycreator"
)

if(NOT IFW_BINARYCREATOR)
    message(FATAL_ERROR "BUILD_INSTALLER=ON but binarycreator not found. Set -DQTIFW_ROOT=<path to QtInstallerFramework/<ver>>.")
endif()
message(STATUS "IFW binarycreator: ${IFW_BINARYCREATOR}")

# app_ifw_var(NAME VALUE): define NAME, NAME_JS (JS literal), NAME_WCPP (C++ wide
# literal) for @ONLY templates. Directory scope only. No newlines/tabs/bare @...@.
function(app_ifw_var name value)
    set(${name} "${value}" PARENT_SCOPE)
    string(REPLACE "\\" "\\\\" _js "${value}")
    string(REPLACE "\"" "\\\"" _js "${_js}")
    set(${name}_JS "\"${_js}\"" PARENT_SCOPE)
    set(${name}_WCPP "L\"${_js}\"" PARENT_SCOPE)
endfunction()

# IFW runtime tokens: round-trip them through configure_file untouched.
set(TargetDir "@TargetDir@")
set(ApplicationsDirX64 "@ApplicationsDirX64@")
set(StartMenuDir "@StartMenuDir@")
set(DesktopDir "@DesktopDir@")

app_ifw_var(APP_DISPLAY_NAME       "${APPLICATION_NAME}")
app_ifw_var(APP_DESCRIPTION        "Desktop sync client.")
app_ifw_var(APP_PUBLISHER          "${APPLICATION_VENDOR}")
app_ifw_var(APP_VERSION            "${MIRALL_VERSION_FULL}")
set(_maint_name "PersonalCloudMaintenanceTool")
app_ifw_var(MAINTENANCE_TOOL_NAME  "${_maint_name}")

app_ifw_var(COMPONENT_MAIN_ID        "com.personalcloud.desktopclient")
app_ifw_var(COMPONENT_SHELL_ID       "com.personalcloud.desktopclient.shellintegration")
app_ifw_var(COMPONENT_START_MENU_ID  "com.personalcloud.desktopclient.startmenushortcut")
app_ifw_var(COMPONENT_DESKTOP_ID     "com.personalcloud.desktopclient.desktopshortcut")
app_ifw_var(COMPONENT_FINDER_ID      "com.personalcloud.desktopclient.finderintegration")
app_ifw_var(COMPONENT_MAINTENANCE_ID "com.personalcloud.maintenancetool")

# Release date: -DAPP_RELEASE_DATE=YYYY-MM-DD from CI; default to version year.
if(NOT DEFINED APP_RELEASE_DATE)
    set(APP_RELEASE_DATE "${MIRALL_VERSION_YEAR}-01-01")
endif()
app_ifw_var(APP_RELEASE_DATE          "${APP_RELEASE_DATE}")
app_ifw_var(APP_SHORTCUT_RELEASE_DATE "${APP_RELEASE_DATE}")

# Windows product vars
app_ifw_var(WINDOWS_INSTALLER_TITLE            "Personal Cloud Files Setup Wizard")
app_ifw_var(WINDOWS_WIZARD_STYLE               "Modern")
app_ifw_var(WINDOWS_RUN_PROGRAM_DESCRIPTION    "Launch Personal Cloud Files after finish")
app_ifw_var(WINDOWS_EXECUTABLE                 "${APPLICATION_EXECUTABLE}.exe")
app_ifw_var(WINDOWS_PROCESS_NAME               "${APPLICATION_EXECUTABLE}")
app_ifw_var(WINDOWS_MAINTENANCE_TOOL_EXECUTABLE "${_maint_name}.exe")
app_ifw_var(WINDOWS_INSTALLER_META_FILE        "PersonalCloud.installer-meta.ini")
app_ifw_var(WINDOWS_INSTALL_DIR                "${APPLICATION_VENDOR}/${APPLICATION_NAME}")
app_ifw_var(WINDOWS_USER_DATA_DIR              "${APPLICATION_SHORTNAME}")
app_ifw_var(WINDOWS_PROTOCOL_SCHEME            "oc")
app_ifw_var(WINDOWS_UNINSTALL_REGISTRY_KEY     "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\PersonalCloudFiles")
# "personalcloud" = installer icon asset, NOT the app icon (pcf). Do not unify.
app_ifw_var(WINDOWS_INSTALLER_APPLICATION_ICON "personalcloud")
# Window icon = multi-res personalcloud.ico; needs the qico plugin
app_ifw_var(WINDOWS_INSTALLER_WINDOW_ICON      "personalcloud.ico")
app_ifw_var(WINDOWS_SHORTCUT_ICON              "personalcloud.ico")
app_ifw_var(WINDOWS_WATERMARK                  "")
# Banner MUST stay empty: a Banner fixes the header width (window can't widen);
# the right-anchored Logo expands instead.
app_ifw_var(WINDOWS_BANNER                     "")
app_ifw_var(WINDOWS_LOGO                       "header_logo.png")

app_ifw_var(WINDOWS_WIZARD_SHOW_PAGE_LIST "true")

# Emit each header pixmap element only when its image is set (omit otherwise).
if(WINDOWS_WATERMARK STREQUAL "")
    set(WINDOWS_WATERMARK_ELEMENT "")
else()
    set(WINDOWS_WATERMARK_ELEMENT "    <Watermark>${WINDOWS_WATERMARK}</Watermark>")
endif()
if(WINDOWS_BANNER STREQUAL "")
    set(WINDOWS_BANNER_ELEMENT "")
else()
    set(WINDOWS_BANNER_ELEMENT "    <Banner>${WINDOWS_BANNER}</Banner>")
endif()
if(WINDOWS_LOGO STREQUAL "")
    set(WINDOWS_LOGO_ELEMENT "")
else()
    set(WINDOWS_LOGO_ELEMENT "    <Logo>${WINDOWS_LOGO}</Logo>")
endif()
set(WINDOWS_STYLESHEET_ELEMENT "")
app_ifw_var(WINDOWS_USE_CUSTOM_STYLESHEET "")
app_ifw_var(WINDOWS_LIGHT_STYLESHEET       "")
app_ifw_var(WINDOWS_DARK_STYLESHEET        "")

# macOS product vars
app_ifw_var(MACOS_INSTALLER_TITLE         "Personal Cloud Files Installer")
app_ifw_var(MACOS_RUN_PROGRAM_DESCRIPTION "Launch Personal Cloud Files after installation")
app_ifw_var(MACOS_PAGE_LIST_PIXMAP        "pagelist.png")
app_ifw_var(MACOS_INSTALL_DIR             "Personal Cloud Files")
app_ifw_var(MACOS_BUNDLE_NAME             "${APPLICATION_EXECUTABLE}.app")
app_ifw_var(MACOS_BUNDLE_EXECUTABLE       "${APPLICATION_EXECUTABLE}")
app_ifw_var(MACOS_USER_DATA_DIR           "${APPLICATION_SHORTNAME}")
app_ifw_var(MACOS_FINDER_SYNC_EXTENSION_ID "com.seagate.personalcloud.stxfiles.macos.FinderSyncExt")

set(IFW_SRC "${CMAKE_CURRENT_SOURCE_DIR}/ifw")   # module is included from admin/
set(IFW_OUT "${CMAKE_BINARY_DIR}/ifw")

if(WIN32)
    set(IFW_OS windows)
elseif(APPLE)
    set(IFW_OS macos)
endif()

if(NOT EXISTS "${IFW_SRC}/templates/${IFW_OS}/config.xml.in")
    message(FATAL_ERROR "IFW templates not found at ${IFW_SRC}/templates/${IFW_OS}")
endif()

file(MAKE_DIRECTORY "${IFW_OUT}/config")
configure_file("${IFW_SRC}/templates/${IFW_OS}/config.xml.in"       "${IFW_OUT}/config/config.xml"       @ONLY)
configure_file("${IFW_SRC}/templates/${IFW_OS}/controlscript.qs.in" "${IFW_OUT}/config/controlscript.qs" @ONLY)

# Static config assets (icons, logo, pagelist) copied verbatim.
file(GLOB _ifw_cfg_assets "${IFW_SRC}/config/${IFW_OS}/*")
file(COPY ${_ifw_cfg_assets} DESTINATION "${IFW_OUT}/config")

if(WIN32)
    file(COPY "${IFW_SRC}/ui" DESTINATION "${IFW_OUT}/config")
endif()

# Each package: meta/*.in -> configured; meta/* non-.in -> copied; data/ created empty.
set(IFW_PACKAGES
    com.personalcloud.desktopclient
    com.personalcloud.desktopclient.shellintegration
    com.personalcloud.desktopclient.startmenushortcut
    com.personalcloud.desktopclient.desktopshortcut
    com.personalcloud.desktopclient.finderintegration
    com.personalcloud.maintenancetool
)

foreach(pkg ${IFW_PACKAGES})
    set(_src_meta "${IFW_SRC}/packages/${pkg}/meta")
    set(_dst_meta "${IFW_OUT}/packages/${pkg}/meta")
    file(MAKE_DIRECTORY "${_dst_meta}")
    file(MAKE_DIRECTORY "${IFW_OUT}/packages/${pkg}/data")

    file(GLOB _meta_files "${_src_meta}/*")
    foreach(f ${_meta_files})
        get_filename_component(_fn "${f}" NAME)
        if(_fn MATCHES "\\.in$")
            string(REGEX REPLACE "\\.in$" "" _out "${_fn}")
            configure_file("${f}" "${_dst_meta}/${_out}" @ONLY)
        else()
            file(COPY "${f}" DESTINATION "${_dst_meta}")
        endif()
    endforeach()
endforeach()

if(WIN32)
    set(_tools_out "${CMAKE_BINARY_DIR}/ifw_tools/InstallerTools")
    file(MAKE_DIRECTORY "${_tools_out}")

    configure_file(
        "${IFW_SRC}/tools/windows-appreg/WindowsAppRegistrar.cpp.in"
        "${CMAKE_BINARY_DIR}/ifw_tools/WindowsAppRegistrar.cpp" @ONLY)
    add_executable(WindowsAppRegistrar "${CMAKE_BINARY_DIR}/ifw_tools/WindowsAppRegistrar.cpp")

    add_executable(ShellExtensionRegistrar
        "${IFW_SRC}/tools/windows-shellextreg/ShellExtensionRegistrar.cpp")
    target_link_libraries(ShellExtensionRegistrar PRIVATE shlwapi shell32 advapi32 ole32)

    # Stage the exes OUTSIDE package data/ (which gets wiped). Pin all per-config
    # output dirs to one path so multi-config generators don't append /Release etc.
    foreach(_t WindowsAppRegistrar ShellExtensionRegistrar)
        set_target_properties(${_t} PROPERTIES
            EXCLUDE_FROM_ALL ON
            RUNTIME_OUTPUT_DIRECTORY                "${_tools_out}"
            RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${_tools_out}"
            RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${_tools_out}"
            RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${_tools_out}"
            RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL     "${_tools_out}")
    endforeach()
endif()

# macOS code-signing + notarization (APPLE + MACOS_SIGN only; Windows unaffected).
if(APPLE)
    option(MACOS_SIGN "codesign + notarize the macOS installer" OFF)
    set(MACOS_APP_CERT       "Developer ID Application: Noveo Inc. (HVE639V94N)" CACHE STRING "codesign Developer ID Application identity")
    set(MACOS_INSTALLER_CERT "Developer ID Installer: Noveo Inc. (HVE639V94N)"  CACHE STRING "Developer ID Installer identity")
    set(MACOS_NOTARY_PROFILE "notary-profile" CACHE STRING "notarytool keychain profile name")
endif()

set(IFW_STAGE "${CMAKE_BINARY_DIR}/ifw_stage" CACHE PATH "Deployed app tree to package")
set(_main_data "${IFW_OUT}/packages/${COMPONENT_MAIN_ID}/data")

if(WIN32)
    set(_installer_name "PersonalCloud-installer.exe")
    set(_installer_deps WindowsAppRegistrar ShellExtensionRegistrar)
else()
    set(_installer_name "PersonalCloud-installer")
    set(_installer_deps "")
endif()

# installer target (NOT in ALL): install -> IFW_STAGE, wipe + repopulate main
# package data/, copy helper tools (Windows), then run binarycreator.
set(_installer_cmds
    COMMAND "${CMAKE_COMMAND}" -E rm -rf "${IFW_STAGE}"
    COMMAND "${CMAKE_COMMAND}" --install "${CMAKE_BINARY_DIR}" --prefix "${IFW_STAGE}" --config $<CONFIG>
    COMMAND "${CMAKE_COMMAND}" -E rm -rf "${_main_data}"
)
if(WIN32)
    list(APPEND _installer_cmds
        COMMAND "${CMAKE_COMMAND}" -E copy_directory "${IFW_STAGE}/bin" "${_main_data}"
    )
elseif(APPLE)
    list(APPEND _installer_cmds
        COMMAND "${CMAKE_COMMAND}" -E copy_directory
                "${IFW_STAGE}/${APPLICATION_EXECUTABLE}.app"
                "${_main_data}/${APPLICATION_EXECUTABLE}.app"
    )
endif()
if(WIN32)
    list(APPEND _installer_cmds
        COMMAND "${CMAKE_COMMAND}" -E copy_directory
                "${CMAKE_BINARY_DIR}/ifw_tools/InstallerTools" "${_main_data}/InstallerTools"
    )
endif()

# Shell-ext DLLs ship in InstallerTools/ShellExtensionsPayload/, NOT the install
# root: ShellExtensionRegistrar deploys+registers them with locked-DLL reboot
# handling. Move them out of data/ root (else `apply` exits 1).
if(WIN32)
    list(APPEND _installer_cmds
        COMMAND "${CMAKE_COMMAND}" -E make_directory
                "${_main_data}/InstallerTools/ShellExtensionsPayload"
        COMMAND "${CMAKE_COMMAND}" -E copy
                "${IFW_STAGE}/bin/CUContextMenu.dll" "${IFW_STAGE}/bin/CUOverlays.dll"
                "${_main_data}/InstallerTools/ShellExtensionsPayload/"
        COMMAND "${CMAKE_COMMAND}" -E rm -f
                "${_main_data}/CUContextMenu.dll" "${_main_data}/CUOverlays.dll"
    )
endif()

# Ship the shortcut icon into the install dir (shortcuts reference
# @TargetDir@/personalcloud.ico, which is not part of the app payload).
if(WIN32)
    list(APPEND _installer_cmds
        COMMAND "${CMAKE_COMMAND}" -E copy
                "${IFW_SRC}/config/${IFW_OS}/${WINDOWS_SHORTCUT_ICON}"
                "${_main_data}/${WINDOWS_SHORTCUT_ICON}"
    )
endif()

# Sign the payload app bundle BEFORE binarycreator packs it (so the installed app
# is signed). --deep does NOT apply the FinderSync .appex entitlements: validate on
# the mac node and switch to per-item + --entitlements signing if the appex fails.
if(APPLE AND MACOS_SIGN)
    list(APPEND _installer_cmds
        COMMAND codesign --force --deep --options runtime --timestamp
                --sign "${MACOS_APP_CERT}"
                "${_main_data}/${APPLICATION_EXECUTABLE}.app"
    )
endif()

add_custom_target(installer
    ${_installer_cmds}
    COMMAND "${IFW_BINARYCREATOR}"
            --config "${IFW_OUT}/config/config.xml"
            --packages "${IFW_OUT}/packages"
            "${CMAKE_BINARY_DIR}/${_installer_name}"
    WORKING_DIRECTORY "${IFW_OUT}"
    DEPENDS ${_installer_deps}
    COMMENT "Building Qt IFW installer -> ${_installer_name}"
    VERBATIM
)

# Sign + notarize the installer binary after binarycreator (mac node: notarytool
# needs a configured keychain profile; stapler may need the binary wrapped first).
if(APPLE AND MACOS_SIGN)
    add_custom_command(TARGET installer POST_BUILD
        COMMAND codesign --force --options runtime --timestamp
                --sign "${MACOS_APP_CERT}" "${CMAKE_BINARY_DIR}/${_installer_name}"
        COMMAND xcrun notarytool submit "${CMAKE_BINARY_DIR}/${_installer_name}"
                --keychain-profile "${MACOS_NOTARY_PROFILE}" --wait
        COMMAND xcrun stapler staple "${CMAKE_BINARY_DIR}/${_installer_name}"
        VERBATIM
        COMMENT "Signing + notarizing macOS installer")
endif()

# Publish a versioned (optionally build-numbered) installer + PDBs + build_success.
set(INSTALLER_OUTPUT_DIR "${CMAKE_BINARY_DIR}" CACHE PATH
    "Directory to publish the named installer artifact, debug symbols and build_success marker")

set(_build_number "$ENV{BUILD_NUMBER}")
if(_build_number)
    set(_artifact_base "PersonalCloud_v${MIRALL_VERSION_FULL}_${_build_number}")
else()
    set(_artifact_base "PersonalCloud_v${MIRALL_VERSION_FULL}")
endif()
if(WIN32)
    set(_artifact_name "${_artifact_base}.exe")
    set(_dbg_kind win)
else()
    set(_artifact_name "${_artifact_base}")
    set(_dbg_kind mac)
endif()

add_custom_command(TARGET installer POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${INSTALLER_OUTPUT_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy
            "${CMAKE_BINARY_DIR}/${_installer_name}"
            "${INSTALLER_OUTPUT_DIR}/${_artifact_name}"
    COMMAND "${CMAKE_COMMAND}"
            -DKIND=${_dbg_kind}
            "-DBUILD_DIR=${CMAKE_BINARY_DIR}"
            "-DSTAGE_DIR=${IFW_STAGE}"
            "-DOUT_DIR=${INSTALLER_OUTPUT_DIR}"
            "-DBASE=${_artifact_base}"
            "-DAPP_EXE=${APPLICATION_EXECUTABLE}"
            -P "${PROJECT_SOURCE_DIR}/cmake/scripts/PublishDebugInfo.cmake"
    COMMAND "${CMAKE_COMMAND}" -E touch "${INSTALLER_OUTPUT_DIR}/build_success"
    VERBATIM
    COMMENT "Publishing installer artifact -> ${INSTALLER_OUTPUT_DIR}/${_artifact_name}")

add_custom_command(TARGET installer POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E echo ""
    COMMAND "${CMAKE_COMMAND}" -E echo "============================================================"
    COMMAND "${CMAKE_COMMAND}" -E echo "  Qt IFW installer built successfully"
    COMMAND "${CMAKE_COMMAND}" -E echo "    installer : ${INSTALLER_OUTPUT_DIR}/${_artifact_name}"
    COMMAND "${CMAKE_COMMAND}" -E echo "    payload   : ${_main_data}"
    COMMAND "${CMAKE_COMMAND}" -E echo "    marker    : ${INSTALLER_OUTPUT_DIR}/build_success"
    COMMAND "${CMAKE_COMMAND}" -E echo "============================================================"
    VERBATIM)
