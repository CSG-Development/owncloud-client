add_library(folderwizard STATIC
    folderwizard.cpp
    folderwizardpage.h
    folderwizarddlg.cpp
    folderwizarddlg.h
    folderwizarddlg.ui
    folderwizard_res.qrc
    folderwizardlocalpath.cpp
    folderwizardsourcepage.ui

    folderwizardremotepath.cpp
    folderwizardtargetpage.ui

    folderwizardselectivesync.cpp

    spacespage.cpp
    spacespage.ui
)

target_link_libraries(folderwizard PUBLIC Qt::Widgets libsync PersonalCloudResources customdialogs customui)
set_target_properties(folderwizard PROPERTIES AUTOUIC ON AUTORCC ON)
app_apply_common_target_settings(folderwizard)
