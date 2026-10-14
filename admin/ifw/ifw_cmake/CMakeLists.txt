cmake_minimum_required(VERSION 3.18)
project(ifw_cmake LANGUAGES NONE)

include(ExternalProject)

set(IFW_QT_STATIC_ROOT "" CACHE PATH "Root of the static Qt used to build the Qt Installer Framework")
set(IFW_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../ifw_sources" CACHE PATH "Qt Installer Framework sources")
set(IFW_INSTALL_DIR "${CMAKE_BINARY_DIR}/ifw-root" CACHE PATH "Directory that receives bin/ with the built IFW tools")
set(IFW_GIT_SHA1 "" CACHE STRING "Revision shown by the IFW tools; defaults to the revision of the client repository")
set(IFW_JOBS "" CACHE STRING "Parallel jobs for make on macOS; defaults to the number of logical cores")
option(IFW_USE_JOM "Use jom instead of nmake on Windows" OFF)
option(IFW_ALLOW_ANY_SDK "Do not require the macOS 15 SDK" OFF)

if(NOT IFW_QT_STATIC_ROOT)
    message(FATAL_ERROR "IFW_QT_STATIC_ROOT is not set. Pass -DIFW_QT_STATIC_ROOT=<root of the static Qt>.")
endif()
get_filename_component(IFW_QT_STATIC_ROOT "${IFW_QT_STATIC_ROOT}" ABSOLUTE)
get_filename_component(IFW_SOURCE_DIR "${IFW_SOURCE_DIR}" ABSOLUTE)
get_filename_component(IFW_INSTALL_DIR "${IFW_INSTALL_DIR}" ABSOLUTE)

if(NOT EXISTS "${IFW_SOURCE_DIR}/installerfw.pro")
    message(FATAL_ERROR "installerfw.pro not found in ${IFW_SOURCE_DIR}")
endif()

find_program(IFW_QMAKE NAMES qmake qmake6 PATHS "${IFW_QT_STATIC_ROOT}/bin" NO_DEFAULT_PATH)
if(NOT IFW_QMAKE)
    message(FATAL_ERROR "qmake not found in ${IFW_QT_STATIC_ROOT}/bin")
endif()

if(WIN32)
    set(_ifw_uitools "${IFW_QT_STATIC_ROOT}/lib/Qt6UiTools.lib")
    if(NOT EXISTS "${_ifw_uitools}")
        message(FATAL_ERROR "Qt UiTools is missing: ${_ifw_uitools}. Build qttools against the static Qt.")
    endif()
    if(IFW_USE_JOM)
        find_program(IFW_MAKE NAMES jom)
        set(_ifw_make_args "")
    else()
        find_program(IFW_MAKE NAMES nmake)
        set(_ifw_make_args /NOLOGO)
    endif()
    if(NOT IFW_MAKE)
        message(FATAL_ERROR "nmake/jom not found. Run from a Visual Studio x64 developer environment.")
    endif()
    find_program(IFW_CL NAMES cl)
    if(NOT IFW_CL)
        message(FATAL_ERROR "cl.exe not found. Run from a Visual Studio x64 developer environment.")
    endif()
elseif(APPLE)
    if(NOT EXISTS "${IFW_QT_STATIC_ROOT}/lib/QtUiTools.framework/QtUiTools"
       AND NOT EXISTS "${IFW_QT_STATIC_ROOT}/lib/libQt6UiTools.a")
        message(FATAL_ERROR "Qt UiTools is missing in ${IFW_QT_STATIC_ROOT}/lib. Build qttools against the static Qt.")
    endif()
    if(NOT IFW_ALLOW_ANY_SDK)
        execute_process(COMMAND xcrun --show-sdk-version
            OUTPUT_VARIABLE _ifw_sdk_version OUTPUT_STRIP_TRAILING_WHITESPACE RESULT_VARIABLE _ifw_sdk_result)
        if(NOT _ifw_sdk_result EQUAL 0 OR NOT _ifw_sdk_version MATCHES "^15(\.|$)")
            message(FATAL_ERROR "macOS 15 SDK (Xcode 16.x) is required, found '${_ifw_sdk_version}'. Select it with DEVELOPER_DIR or pass -DIFW_ALLOW_ANY_SDK=ON.")
        endif()
    endif()
    find_program(IFW_MAKE NAMES make)
    if(NOT IFW_MAKE)
        message(FATAL_ERROR "make not found")
    endif()
    if(NOT IFW_JOBS)
        cmake_host_system_information(RESULT IFW_JOBS QUERY NUMBER_OF_LOGICAL_CORES)
    endif()
    set(_ifw_make_args -j${IFW_JOBS})
else()
    message(FATAL_ERROR "Only Windows and macOS are supported")
endif()

if(NOT IFW_GIT_SHA1)
    get_filename_component(_ifw_client_root "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
    find_package(Git QUIET)
    if(GIT_FOUND)
        execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
            WORKING_DIRECTORY "${_ifw_client_root}"
            OUTPUT_VARIABLE IFW_GIT_SHA1 OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET RESULT_VARIABLE _ifw_git_result)
        if(NOT _ifw_git_result EQUAL 0)
            set(IFW_GIT_SHA1 "")
        endif()
    endif()
    if(NOT IFW_GIT_SHA1 AND EXISTS "${_ifw_client_root}/.tag")
        file(READ "${_ifw_client_root}/.tag" IFW_GIT_SHA1)
        string(REPLACE "\n" "" IFW_GIT_SHA1 "${IFW_GIT_SHA1}")
        if(IFW_GIT_SHA1 STREQUAL "$Format:%H$")
            set(IFW_GIT_SHA1 "")
        endif()
    endif()
endif()
message(STATUS "IFW_GIT_SHA1 ${IFW_GIT_SHA1}")

set(_ifw_qmake_args CONFIG+=release CONFIG-=debug "GIT_SHA1=${IFW_GIT_SHA1}")
if(APPLE AND CMAKE_OSX_ARCHITECTURES)
    string(REPLACE ";" " " _ifw_archs "${CMAKE_OSX_ARCHITECTURES}")
    list(APPEND _ifw_qmake_args "QMAKE_APPLE_DEVICE_ARCHS=${_ifw_archs}")
endif()

ExternalProject_Add(ifw
    SOURCE_DIR "${IFW_SOURCE_DIR}"
    BINARY_DIR "${CMAKE_BINARY_DIR}/ifw-qmake"
    STAMP_DIR "${CMAKE_BINARY_DIR}/ifw-stamp"
    TMP_DIR "${CMAKE_BINARY_DIR}/ifw-tmp"
    CONFIGURE_COMMAND "${IFW_QMAKE}" "<SOURCE_DIR>/installerfw.pro" ${_ifw_qmake_args}
    BUILD_COMMAND "${IFW_MAKE}" ${_ifw_make_args}
    BUILD_ALWAYS TRUE
    INSTALL_COMMAND "${CMAKE_COMMAND}" -E copy_directory "<BINARY_DIR>/bin" "${IFW_INSTALL_DIR}/bin"
    USES_TERMINAL_CONFIGURE TRUE
    USES_TERMINAL_BUILD TRUE
)
