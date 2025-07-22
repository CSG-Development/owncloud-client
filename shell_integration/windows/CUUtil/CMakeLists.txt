add_library(CUUtil STATIC
    CommunicationSocket.cpp
    RemotePathChecker.cpp
    StringUtil.cpp
 )

target_include_directories(CUUtil
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)
