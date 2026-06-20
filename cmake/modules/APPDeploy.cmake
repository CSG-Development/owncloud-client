# ---------------------------------------------------------------------------
# app_deploy_runtime(<target>)
# ---------------------------------------------------------------------------
function(app_deploy_runtime target)
    if(WIN32)
        # ------------------------------------------------------------------
        # Locate windeployqt6.
        #
        # Strategy (in priority order):
        #   1. $ENV{QT_ROOT}/bin  — set in CMakePresets.json via $env{QT_ROOT}
        #   2. Derive from Qt6::qmake IMPORTED_LOCATION at configure time
        #   3. System PATH fallback
        # ------------------------------------------------------------------
        find_program(_wdqt
            NAMES windeployqt6 windeployqt
            HINTS "$ENV{QT_ROOT}/bin"
            NO_DEFAULT_PATH
        )
        if(NOT _wdqt)
            get_target_property(_qmake_loc Qt6::qmake IMPORTED_LOCATION)
            if(_qmake_loc)
                get_filename_component(_qt_bin_dir "${_qmake_loc}" DIRECTORY)
                find_program(_wdqt
                    NAMES windeployqt6 windeployqt
                    HINTS "${_qt_bin_dir}"
                    NO_DEFAULT_PATH
                )
            endif()
        endif()
        if(NOT _wdqt)
            find_program(_wdqt NAMES windeployqt6 windeployqt)
        endif()
        if(NOT _wdqt)
            message(FATAL_ERROR
                "app_deploy_runtime: windeployqt6 not found. "
                "Set QT_ROOT env var to your Qt installation root "
                "(e.g. S:/Qt/6.8.3/msvc2022_64).")
        endif()
        message(STATUS "app_deploy_runtime: windeployqt = ${_wdqt}")

        # ------------------------------------------------------------------
        # 1. Run windeployqt on the installed exe at install time.
        # ------------------------------------------------------------------
        install(CODE "
            message(STATUS \"Running windeployqt on \${CMAKE_INSTALL_PREFIX}/bin/$<TARGET_FILE_NAME:${target}>\")
            execute_process(
                COMMAND \"${_wdqt}\"
                        \"\${CMAKE_INSTALL_PREFIX}/bin/$<TARGET_FILE_NAME:${target}>\"
                        $<IF:$<CONFIG:Debug>,--debug,--release>
                        --no-compiler-runtime
                        --no-translations
                COMMAND_ERROR_IS_FATAL ANY
            )
        ")

        # ------------------------------------------------------------------
        # 2. Install non-Qt runtime DLLs resolved from the CMake link graph.
        #    $<TARGET_RUNTIME_DLLS:tgt> expands to the full paths of DLLs
        #    that back IMPORTED SHARED targets in the link closure.
        #    Note: UNKNOWN IMPORTED targets (ZLIB, SQLite3, set via -DZLIB_LIBRARY=
        #    pointing to a .lib) are not picked up here — handled explicitly below.
        # ------------------------------------------------------------------
        install(FILES $<TARGET_RUNTIME_DLLS:${target}>
            DESTINATION bin
        )

        # ------------------------------------------------------------------
        # 3. Explicitly install DLLs that CMake cannot resolve via
        #    TARGET_RUNTIME_DLLS because they are found as UNKNOWN IMPORTED
        #    targets (their .lib is set directly in cache, not via a proper
        #    SHARED IMPORTED config that exposes IMPORTED_IMPLIB + IMPORTED_LOCATION).
        #
        #    OpenSSL: DLLs live in <OPENSSL_ROOT_DIR>/bin/
        #    ZLIB:    DLL lives next to the import lib (replace .lib → .dll),
        #             but only if it is actually a shared library.
        #    SQLite3: same pattern — .lib may be an import lib for sqlite3.dll.
        # ------------------------------------------------------------------

        # OpenSSL — always shared on Windows in this deps tree.
        if(OPENSSL_ROOT_DIR)
            file(GLOB _openssl_dlls "${OPENSSL_ROOT_DIR}/bin/libcrypto-*.dll" "${OPENSSL_ROOT_DIR}/bin/libssl-*.dll")
            if(_openssl_dlls)
                install(FILES ${_openssl_dlls} DESTINATION bin)
            endif()
        endif()

        # ZLIB — derive DLL from import lib path.
        if(ZLIB_LIBRARY)
            get_filename_component(_zlib_lib_dir "${ZLIB_LIBRARY}" DIRECTORY)
            get_filename_component(_zlib_lib_name "${ZLIB_LIBRARY}" NAME_WE)  # e.g. "zlib"
            # DLL is typically in the sibling bin/ directory.
            if(CMAKE_BUILD_TYPE STREQUAL "Debug")
                file(GLOB _zlib_dlls "${_zlib_lib_dir}/../bin/${_zlib_lib_name}d.dll")
            else()
                file(GLOB _zlib_dlls "${_zlib_lib_dir}/../bin/${_zlib_lib_name}.dll")
            endif()
            if(_zlib_dlls)
                install(FILES ${_zlib_dlls} DESTINATION bin)
            endif()
        endif()

        # SQLite3 — derive DLL from import lib path.
        if(SQLite3_LIBRARY)
            get_filename_component(_sqlite_lib_dir "${SQLite3_LIBRARY}" DIRECTORY)
            get_filename_component(_sqlite_lib_name "${SQLite3_LIBRARY}" NAME_WE)  # e.g. "sqlite3"
            file(GLOB _sqlite_dlls
                "${_sqlite_lib_dir}/../bin/${_sqlite_lib_name}.dll"
            )
            if(_sqlite_dlls)
                install(FILES ${_sqlite_dlls} DESTINATION bin)
            else()
                # sqlite3.lib is an import lib -> the app needs sqlite3.dll at runtime.
                # Warn at configure rather than ship a package that crashes on launch
                # with "sqlite3.dll was not found".
                message(WARNING "app_deploy_runtime: sqlite3 runtime DLL not found at "
                    "${_sqlite_lib_dir}/../bin/${_sqlite_lib_name}.dll. The installed app will "
                    "fail to start (sqlite3.dll missing). Put sqlite3.dll in the deps bin/ dir.")
            endif()
        endif()

    elseif(APPLE)
        # ------------------------------------------------------------------
        # Locate macdeployqt.
        #
        # Strategy (in priority order):
        #   1. $ENV{QT_ROOT}/bin
        #   2. Derive from Qt6::qmake IMPORTED_LOCATION at configure time
        # ------------------------------------------------------------------
        find_program(_mdqt
            NAMES macdeployqt
            HINTS "$ENV{QT_ROOT}/bin"
            NO_DEFAULT_PATH
        )
        if(NOT _mdqt)
            get_target_property(_qmake_loc Qt6::qmake IMPORTED_LOCATION)
            if(_qmake_loc)
                get_filename_component(_qt_bin_dir "${_qmake_loc}" DIRECTORY)
                find_program(_mdqt
                    NAMES macdeployqt
                    HINTS "${_qt_bin_dir}"
                    NO_DEFAULT_PATH
                )
            endif()
        endif()
        if(NOT _mdqt)
            message(FATAL_ERROR
                "app_deploy_runtime: macdeployqt not found. "
                "Set QT_ROOT env var to your Qt installation root "
                "(e.g. /opt/Qt/6.8.3/macos).")
        endif()
        message(STATUS "app_deploy_runtime: macdeployqt = ${_mdqt}")

        # Capture the deps prefix at configure time so it is baked into
        # the install(CODE) string (generator expressions are not available
        # inside install(CODE) scripts).
        set(_deps_root "$ENV{DEPS_ROOT}")
        if(NOT _deps_root)
            message(WARNING "DEPS_ROOT is not set; libkdsingleapplication-qt6 and libqt6keychain will not be bundled into the .app.")
        endif()
        set(_qt_root   "$ENV{QT_ROOT}")

        # Directory of the zlib CMake actually resolved (ZLIB::ZLIB). The deps
        # libz ships with an @executable_path/../Frameworks install id, so
        # macdeployqt treats it as already-bundled and skips it — we must copy
        # it ourselves. Derive from ZLIB_LIBRARY (not DEPS_ROOT/lib) so this
        # works regardless of where the resolved zlib lives.
        get_filename_component(_zlib_dir "${ZLIB_LIBRARY}" DIRECTORY)

        # ------------------------------------------------------------------
        # All steps run at cmake --install time via install(CODE).
        # The bundle is installed to the prefix root (BUNDLE DESTINATION ".").
        # ------------------------------------------------------------------
        install(CODE "
            set(_bundle \"\${CMAKE_INSTALL_PREFIX}/${APPLICATION_EXECUTABLE}.app\")
            message(STATUS \"macOS deploy: running macdeployqt on \${_bundle}\")

            # 1. macdeployqt — embeds Qt frameworks and performs @rpath relocation.
            #    -libpath points at the install lib dir so macdeployqt can resolve the
            #    project's own @rpath dylibs (libPersonalCloudsync / _csync / Resources),
            #    copy them into Contents/Frameworks and rewrite their install names.
            execute_process(
                COMMAND \"${_mdqt}\" \"\${_bundle}\" -always-overwrite \"-libpath=\${CMAKE_INSTALL_PREFIX}/lib\"
                COMMAND_ERROR_IS_FATAL ANY
            )

            # 2. Copy frameworks that macdeployqt misses.
            set(_fw_src \"${_qt_root}/lib\")
            set(_fw_dst \"\${_bundle}/Contents/Frameworks\")
            foreach(_fw QtOpenGL QtOpenGLWidgets QtSvgWidgets)
                set(_src_fw \"\${_fw_src}/\${_fw}.framework\")
                if(EXISTS \"\${_src_fw}\")
                    message(STATUS \"macOS deploy: copying \${_fw}.framework\")
                    file(COPY \"\${_src_fw}\" DESTINATION \"\${_fw_dst}\")
                else()
                    message(STATUS \"macOS deploy: \${_fw}.framework not found at \${_src_fw} (skipping)\")
                endif()
            endforeach()

            # 3. Copy extra dylibs from the deps prefix (libkdsingleapplication, libqt6keychain).
            set(_deps_lib \"${_deps_root}/lib\")
            if(IS_DIRECTORY \"\${_deps_lib}\")
                file(GLOB _kd_dylibs  \"\${_deps_lib}/libkdsingleapplication-qt6*.dylib\")
                file(GLOB _kc_dylibs  \"\${_deps_lib}/libqt6keychain*.dylib\")
                foreach(_dylib IN LISTS _kd_dylibs _kc_dylibs)
                    get_filename_component(_dname \"\${_dylib}\" NAME)
                    message(STATUS \"macOS deploy: copying \${_dname}\")
                    file(COPY \"\${_dylib}\" DESTINATION \"\${_fw_dst}\")
                endforeach()
            else()
                message(STATUS \"macOS deploy: DEPS_ROOT/lib not found (\${_deps_lib}); extra dylibs skipped\")
            endif()

            # 3b. Copy the resolved zlib (libz*.dylib). macdeployqt skips it
            #     because its install id is @executable_path/../Frameworks/...
            #     so without this the app aborts at launch (libz... not found).
            if(IS_DIRECTORY \"${_zlib_dir}\")
                file(GLOB _z_dylibs \"${_zlib_dir}/libz*.dylib\")
                foreach(_dylib IN LISTS _z_dylibs)
                    get_filename_component(_dname \"\${_dylib}\" NAME)
                    message(STATUS \"macOS deploy: copying \${_dname}\")
                    file(COPY \"\${_dylib}\" DESTINATION \"\${_fw_dst}\")
                endforeach()
            else()
                message(WARNING \"macOS deploy: zlib dir not found (${_zlib_dir}); libz will be missing from the bundle\")
            endif()

            message(STATUS \"macOS deploy: ensuring @executable_path/../Frameworks rpath on the executable\")
            execute_process(
                COMMAND install_name_tool -add_rpath \"@executable_path/../Frameworks\"
                        \"\${_bundle}/Contents/MacOS/${APPLICATION_EXECUTABLE}\"
            )

            # 4. Copy Qt base translations and update qt.conf.
            set(_trans_src \"${_qt_root}/translations\")
            set(_trans_dst \"\${_bundle}/Contents/Resources/Translations\")
            file(MAKE_DIRECTORY \"\${_trans_dst}\")

            file(GLOB _qtbase_qm  \"\${_trans_src}/qtbase_*.qm\")
            file(GLOB _qt_2_qm    \"\${_trans_src}/qt_??.qm\")
            file(GLOB _qt_5_qm    \"\${_trans_src}/qt_??_??.qm\")
            foreach(_qm IN LISTS _qtbase_qm _qt_2_qm _qt_5_qm)
                file(COPY \"\${_qm}\" DESTINATION \"\${_trans_dst}\")
            endforeach()

            # Copy qt6keychain translations from the deps prefix.
            file(GLOB _keychain_qm \"${_deps_root}/share/qt6keychain/translations/*\")
            if(_keychain_qm)
                file(COPY \${_keychain_qm} DESTINATION \"\${_trans_dst}\")
            endif()

            # Append Translations entry to qt.conf (only if not already present).
            set(_qtconf \"\${_bundle}/Contents/Resources/qt.conf\")
            if(EXISTS \"\${_qtconf}\")
                file(READ \"\${_qtconf}\" _qtconf_contents)
            else()
                set(_qtconf_contents \"\")
            endif()
            if(NOT _qtconf_contents MATCHES \"Translations =\")
                file(APPEND \"\${_qtconf}\" \"Translations = Resources/Translations\n\")
                message(STATUS \"macOS deploy: appended Translations entry to qt.conf\")
            endif()
        ")
    endif()
    # Other platforms (Linux): no-op.
endfunction()

# ---------------------------------------------------------------------------
# app_deploy_flatten_plugins(<app_shortname>)
#
# Call this from the root CMakeLists.txt AFTER add_subdirectory(src) so the
# plugin install(TARGET ...) rules have already been registered and will execute
# before this install(CODE) block runs during cmake --install.
#
# Copies every DLL from lib/<shortname>/plugins/ into bin/ so the runtime
# loader finds vfs plugins alongside the exe without PATH manipulation.
# ---------------------------------------------------------------------------
function(app_deploy_flatten_plugins app_shortname)
    if(WIN32)
        install(CODE "
            set(_plugin_dir \"\${CMAKE_INSTALL_PREFIX}/lib/${app_shortname}/plugins\")
            set(_bin_dir    \"\${CMAKE_INSTALL_PREFIX}/bin\")
            if(EXISTS \"\${_plugin_dir}\")
                file(GLOB _plugin_dlls \"\${_plugin_dir}/*.dll\")
                foreach(_dll IN LISTS _plugin_dlls)
                    get_filename_component(_name \"\${_dll}\" NAME)
                    message(STATUS \"Deploying plugin: \${_name} -> bin/\")
                    file(COPY \"\${_dll}\" DESTINATION \"\${_bin_dir}\")
                endforeach()
            else()
                message(STATUS \"app_deploy_flatten_plugins: plugin dir not found (skipping): \${_plugin_dir}\")
            endif()
        ")
    endif()
    # APPLE (Task D2) and other platforms: no-op.
endfunction()
