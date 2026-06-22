if(APPLE)
    if (SPARKLE_FOUND)
        list(APPEND APPLICATION_BUNDLED_RESOURCES
            "${PROJECT_SOURCE_DIR}/admin/osx/deny_autoupdate_com.personalcloud.desktopclient.plist"
            "${PROJECT_SOURCE_DIR}/admin/osx/sparkle/dsa_pub.pem")
        set(APPLICATION_BUNDLED_RESOURCES ${APPLICATION_BUNDLED_RESOURCES} PARENT_SCOPE)
    endif()
endif()

target_sources(PersonalCloudCore PRIVATE
    ocupdater.cpp
    ocupdater.h
    updateinfo.cpp
    updateinfo.h
    updater.cpp
    updater.h
)

if(SPARKLE_FOUND)
    # Define this, we need to check in updater.cpp
    target_compile_definitions(PersonalCloudCore PUBLIC HAVE_SPARKLE)
    target_sources(PersonalCloudCore PRIVATE
        sparkleupdater_mac.mm
        sparkleupdater.h
    )
    target_link_libraries(PersonalCloudCore PRIVATE ${SPARKLE_LIBRARY})
endif()
