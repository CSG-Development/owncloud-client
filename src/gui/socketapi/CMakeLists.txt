target_sources(curatorCore PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/socketapi.cpp
    )
    
if( APPLE )
    target_sources(curatorCore PRIVATE socketapisocket_mac.mm)
endif()
