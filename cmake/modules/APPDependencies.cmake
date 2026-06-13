find_package(QT 6.2 NAMES Qt6 COMPONENTS Core REQUIRED)

find_package(Qt6 COMPONENTS Core Concurrent Network Widgets Xml REQUIRED)
find_package(Qt6LinguistTools REQUIRED)
get_target_property(QT_QMAKE_EXECUTABLE Qt::qmake IMPORTED_LOCATION)
message(STATUS "Using Qt ${QT_VERSION} (${QT_QMAKE_EXECUTABLE})")

# Sparkle (macOS auto-updater) intentionally removed: updates are handled by the
# Qt Installer Framework maintenance tool. The default build has WITH_AUTO_UPDATER=OFF
# and no longer depends on Sparkle.

find_package(ZLIB REQUIRED)
find_package(SQLite3 3.9.0 REQUIRED)
