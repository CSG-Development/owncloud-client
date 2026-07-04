if(NOT INSTALLERBASE OR NOT APP_OUT OR NOT TOOL_NAME OR NOT BUNDLE_ID)
    message(FATAL_ERROR "MakeMaintenanceToolApp: need -DINSTALLERBASE= -DAPP_OUT= -DTOOL_NAME= -DBUNDLE_ID=")
endif()

if(NOT EXISTS "${INSTALLERBASE}")
    message(FATAL_ERROR "MakeMaintenanceToolApp: installerbase not found at ${INSTALLERBASE}")
endif()

if(NOT VERSION)
    set(VERSION "1.0.0")
endif()
if(NOT DEPLOYMENT_TARGET)
    set(DEPLOYMENT_TARGET "12.0")
endif()

set(_contents   "${APP_OUT}/Contents")
set(_macos      "${_contents}/MacOS")
set(_resources  "${_contents}/Resources")

file(REMOVE_RECURSE "${APP_OUT}")
file(MAKE_DIRECTORY "${_macos}")
file(MAKE_DIRECTORY "${_resources}")

file(COPY "${INSTALLERBASE}" DESTINATION "${_macos}")
get_filename_component(_ib_name "${INSTALLERBASE}" NAME)
if(NOT _ib_name STREQUAL "${TOOL_NAME}")
    file(RENAME "${_macos}/${_ib_name}" "${_macos}/${TOOL_NAME}")
endif()
execute_process(COMMAND chmod +x "${_macos}/${TOOL_NAME}")

set(_icon_key "")
if(ICNS AND EXISTS "${ICNS}")
    file(COPY "${ICNS}" DESTINATION "${_resources}")
    get_filename_component(_icns_name "${ICNS}" NAME)
    set(_icon_key "    <key>CFBundleIconFile</key>\n    <string>${_icns_name}</string>\n")
endif()

file(WRITE "${_contents}/PkgInfo" "APPL????")

file(WRITE "${_contents}/Info.plist"
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">
<plist version=\"1.0\">
<dict>
    <key>CFBundleExecutable</key>
    <string>${TOOL_NAME}</string>
    <key>CFBundleIdentifier</key>
    <string>${BUNDLE_ID}</string>
    <key>CFBundleName</key>
    <string>${TOOL_NAME}</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleSignature</key>
    <string>????</string>
    <key>CFBundleShortVersionString</key>
    <string>${VERSION}</string>
    <key>CFBundleVersion</key>
    <string>${VERSION}</string>
${_icon_key}    <key>LSMinimumSystemVersion</key>
    <string>${DEPLOYMENT_TARGET}</string>
    <key>NSHighResolutionCapable</key>
    <true/>
    <key>NSPrincipalClass</key>
    <string>NSApplication</string>
</dict>
</plist>
")

if(NOT CERT)
    set(CERT "-")
endif()

if(CERT STREQUAL "-")
    message(STATUS "MakeMaintenanceToolApp: ad-hoc signing ${APP_OUT}")
    execute_process(COMMAND codesign --force --sign - "${APP_OUT}" RESULT_VARIABLE _rc)
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR "MakeMaintenanceToolApp: ad-hoc codesign failed (rc=${_rc})")
    endif()
else()
    set(_ent_file "")
    execute_process(
        COMMAND codesign -d --entitlements - --xml "${INSTALLERBASE}"
        OUTPUT_VARIABLE _ent_xml
        ERROR_QUIET
        RESULT_VARIABLE _ent_rc)
    if(_ent_rc EQUAL 0 AND _ent_xml MATCHES "<key>")
        set(_ent_file "${APP_OUT}.entitlements")
        file(WRITE "${_ent_file}" "${_ent_xml}")
        message(STATUS "MakeMaintenanceToolApp: preserving installerbase entitlements")
    endif()

    set(_sign_cmd codesign --force --options runtime --timestamp --sign "${CERT}")
    if(_ent_file)
        list(APPEND _sign_cmd --entitlements "${_ent_file}")
    endif()
    list(APPEND _sign_cmd "${APP_OUT}")

    message(STATUS "MakeMaintenanceToolApp: signing ${APP_OUT} with '${CERT}'")
    execute_process(COMMAND ${_sign_cmd} RESULT_VARIABLE _rc)
    if(_ent_file)
        file(REMOVE "${_ent_file}")
    endif()
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR "MakeMaintenanceToolApp: codesign failed (rc=${_rc})")
    endif()
endif()

execute_process(
    COMMAND codesign --verify --strict --verbose=2 "${APP_OUT}"
    RESULT_VARIABLE _rc)
if(NOT _rc EQUAL 0)
    message(FATAL_ERROR "MakeMaintenanceToolApp: codesign verify failed (rc=${_rc})")
endif()

if(NOTARIZE AND NOT CERT STREQUAL "-")
    if(NOT NOTARY_PROFILE)
        message(FATAL_ERROR "MakeMaintenanceToolApp: NOTARIZE=ON but NOTARY_PROFILE not set")
    endif()
    set(_zip "${APP_OUT}.zip")
    file(REMOVE "${_zip}")
    execute_process(
        COMMAND ditto -c -k --keepParent "${APP_OUT}" "${_zip}"
        RESULT_VARIABLE _rc)
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR "MakeMaintenanceToolApp: ditto zip failed (rc=${_rc})")
    endif()
    message(STATUS "MakeMaintenanceToolApp: notarizing ${_zip}")
    execute_process(
        COMMAND xcrun notarytool submit "${_zip}"
                --keychain-profile "${NOTARY_PROFILE}" --wait
        RESULT_VARIABLE _rc)
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR "MakeMaintenanceToolApp: notarytool submit failed (rc=${_rc})")
    endif()
    execute_process(COMMAND xcrun stapler staple "${APP_OUT}" RESULT_VARIABLE _rc)
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR "MakeMaintenanceToolApp: stapler staple failed (rc=${_rc})")
    endif()
    file(REMOVE "${_zip}")
endif()

message(STATUS "MakeMaintenanceToolApp: built ${APP_OUT}")
