set( APPLICATION_NAME       "Curator Files" )
set( APPLICATION_SHORTNAME  "Curator" )
set( APPLICATION_EXECUTABLE "curator" )
set( APPLICATION_DOMAIN     "owncloud.com" )
set( APPLICATION_VENDOR     "Seagate Technologies LLC" )
set( APPLICATION_UPDATE_URL "https://updates.owncloud.com/client/" CACHE STRING "URL for updater" )
set( APPLICATION_ICON_NAME  "curator" )
set( APPLICATION_VIRTUALFILE_SUFFIX "curator" CACHE STRING "Virtual file suffix (not including the .)")
set( APPLICATION_BUNDLE_NAME "Curator Files" )

set( LINUX_PACKAGE_SHORTNAME "curator" )

set( THEME_CLASS            "CuratorTheme" )
set( APPLICATION_REV_DOMAIN "com.seagate.curator.stxfiles.macos" )
set( WIN_SETUP_BITMAP_PATH  "${CMAKE_SOURCE_DIR}/admin/win/nsi" )

set( MAC_INSTALLER_BACKGROUND_FILE "${CMAKE_SOURCE_DIR}/admin/osx/installer-background.png" CACHE STRING "The MacOSX installer background image")

# set( THEME_INCLUDE          "${OEM_THEME_DIR}/mytheme.h" )
# set( APPLICATION_LICENSE    "${OEM_THEME_DIR}/license.txt )

option( WITH_CRASHREPORTER "Build crashreporter" OFF )
set( CRASHREPORTER_SUBMIT_URL "https://crash-reports.owncloud.com/submit" CACHE STRING "URL for crash reporter" )
