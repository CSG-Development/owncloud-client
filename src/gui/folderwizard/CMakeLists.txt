add_library(folderwizard STATIC
    folderwizard.cpp
    folderwizardlocalpath.cpp
    folderwizardsourcepage.ui

    folderwizardremotepath.cpp
    folderwizardtargetpage.ui

    folderwizardselectivesync.cpp

    spacespage.cpp
    spacespage.ui
)

if(APPLE)
    string(REPLACE "-fno-exceptions" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    add_compile_options(-fexceptions)
endif()

target_link_libraries(folderwizard PUBLIC Qt::Widgets libsync curatorResources)
set_target_properties(folderwizard PROPERTIES AUTOUIC ON AUTORCC ON)
apply_common_target_settings(folderwizard)
