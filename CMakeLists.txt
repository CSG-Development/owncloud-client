cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(VERSION.cmake)
project(personalcloud-client LANGUAGES CXX C VERSION ${MIRALL_VERSION_MAJOR}.${MIRALL_VERSION_MINOR}.${MIRALL_VERSION_PATCH})
include(FeatureSummary)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)
set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)

if(WIN32)
    add_definitions(-DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -DNOMINMAX)
    add_definitions(
        -D_CRT_SECURE_NO_DEPRECATE
        -D_CRT_SECURE_NO_WARNINGS
        -D_CRT_NONSTDC_NO_DEPRECATE
        -D_SCL_SECURE_NO_WARNINGS
    )
endif()

include(APPPlatform)
include(APPOptions)
include(APPDependencies)
include(CTest)

include(APPApplicationIcon)
include(APPBundleResources)
include(APPApplyCommonSettings)

include("${CMAKE_CURRENT_LIST_DIR}/THEME.cmake")

include(APPInstallDirs)
include(APPDeploy)

set(synclib_NAME "${APPLICATION_LIB_PREFIX}sync")

if(NOT WITH_CRASHREPORTER)
    message(STATUS "Build of crashreporter disabled.")
else()
    # obviously, a crash reported without the URL set is pretty pointless and won't work
    if(NOT DEFINED CRASHREPORTER_SUBMIT_URL)
        message(FATAL_ERROR "No crash reporter submit URL provided")
    endif()
endif()

include(GenerateExportHeader)

include(APPGitRevision)

if (WITH_CAP_CORE)
    include(CapCore)
endif (WITH_CAP_CORE)

include(APPBrandingResources)

if(BUILD_INSTALLER)
    add_subdirectory(admin)
endif()

add_subdirectory(src)

app_deploy_flatten_plugins("${APPLICATION_SHORTNAME}")

if(BUILD_INSTALLER AND TARGET installer)
    if(TARGET PersonalCloud)
        add_dependencies(installer PersonalCloud)
    endif()
    foreach(_vfs ${VIRTUAL_FILE_SYSTEM_PLUGINS})
        if(TARGET vfs_${_vfs})
            add_dependencies(installer vfs_${_vfs})
        endif()
    endforeach()
endif()

if(BUILD_SHELL_INTEGRATION)
    add_subdirectory(shell_integration)
endif()

if(BUILD_TESTING)
    add_subdirectory(test)
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
