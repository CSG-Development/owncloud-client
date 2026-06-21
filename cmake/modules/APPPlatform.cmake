if(NOT WIN32 AND NOT APPLE)
    message(FATAL_ERROR "Only Windows and macOS builds are supported.")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
