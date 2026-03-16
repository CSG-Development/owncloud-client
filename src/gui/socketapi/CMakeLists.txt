target_sources(PersonalCloudCore PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/socketapi.cpp
    )
    
if( APPLE )
    target_sources(PersonalCloudCore PRIVATE socketapisocket_mac.mm)
endif()
