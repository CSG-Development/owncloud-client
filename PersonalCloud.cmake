set( APPLICATION_NAME       "Personal Cloud Files" )
set( APPLICATION_SHORTNAME  "PersonalCloud" )
set( APPLICATION_EXECUTABLE "PersonalCloud" )
set( APPLICATION_DOMAIN     "owncloud.com" )
set( APPLICATION_VENDOR     "Seagate Technologies LLC" )
set( APPLICATION_UPDATE_URL "https://updates.owncloud.com/client/" CACHE STRING "URL for updater" )
set( APPLICATION_LIB_PREFIX "PersonalCloud" )
set( APPLICATION_ICON_NAME  "pcf" )
set( APPLICATION_VIRTUALFILE_SUFFIX "PersonalCloud" CACHE STRING "Virtual file suffix (not including the .)")
set( APPLICATION_BUNDLE_NAME "Personal Cloud" )

set( LINUX_PACKAGE_SHORTNAME "PersonalCloud" )

set( THEME_CLASS            "ApplicationTheme" )
set( APPLICATION_REV_DOMAIN "com.seagate.personalcloud.stxfiles.macos" )
set( WIN_SETUP_BITMAP_PATH  "${CMAKE_SOURCE_DIR}/admin/win/nsi" )

set( MAC_INSTALLER_BACKGROUND_FILE "${CMAKE_SOURCE_DIR}/admin/osx/installer-background.png" CACHE STRING "The MacOSX installer background image")

# set( THEME_INCLUDE          "${OEM_THEME_DIR}/mytheme.h" )
# set( APPLICATION_LICENSE    "${OEM_THEME_DIR}/license.txt )

option( WITH_CRASHREPORTER "Build crashreporter" OFF )
set( CRASHREPORTER_SUBMIT_URL "https://crash-reports.owncloud.com/submit" CACHE STRING "URL for crash reporter" )
