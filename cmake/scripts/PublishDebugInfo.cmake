# Package debug symbols for the installer artifact.
# Invoked at installer build time via:
#   cmake -DKIND=win|mac -DBUILD_DIR=... -DSTAGE_DIR=... -DOUT_DIR=...
#         -DBASE=... -DAPP_EXE=... -P PublishDebugInfo.cmake
#
# Windows: zips all *.pdb found in the build tree -> <BASE>_pdb.zip
# macOS:   runs dsymutil on the staged app binary -> <BASE>_dSYM.zip

if(KIND STREQUAL "win")
    file(GLOB_RECURSE _pdbs "${BUILD_DIR}/*.pdb")
    if(_pdbs)
        file(ARCHIVE_CREATE
            OUTPUT "${OUT_DIR}/${BASE}_pdb.zip"
            PATHS ${_pdbs}
            FORMAT zip)
        message(STATUS "debuginfo: ${OUT_DIR}/${BASE}_pdb.zip")
    else()
        message(STATUS "debuginfo: no .pdb files found under ${BUILD_DIR}")
    endif()
elseif(KIND STREQUAL "mac")
    set(_exe "${STAGE_DIR}/${APP_EXE}.app/Contents/MacOS/${APP_EXE}")
    if(EXISTS "${_exe}")
        execute_process(
            COMMAND dsymutil "${_exe}" -o "${OUT_DIR}/${BASE}.dSYM"
            RESULT_VARIABLE _rc)
        if(_rc EQUAL 0 AND EXISTS "${OUT_DIR}/${BASE}.dSYM")
            file(ARCHIVE_CREATE
                OUTPUT "${OUT_DIR}/${BASE}_dSYM.zip"
                PATHS "${OUT_DIR}/${BASE}.dSYM"
                FORMAT zip)
            message(STATUS "debuginfo: ${OUT_DIR}/${BASE}_dSYM.zip")
        else()
            message(STATUS "debuginfo: dsymutil failed (rc=${_rc}); skipping dSYM archive")
        endif()
    else()
        message(STATUS "debuginfo: app binary not found at ${_exe}; skipping dSYM")
    endif()
endif()
