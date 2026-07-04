add_library(onboarding STATIC
    onboardingcontroller.h
    onboardingcontroller.cpp
    onboardingstate.h
    onboardingstate.cpp
    onboardingdlg.cpp
    onboardingdlg.h
    onboardingdlg.ui
    onboardingindicator.cpp
    onboardingindicator.h
    onboardingpage1.cpp
    onboardingpage1.h
    onboardingpage1.ui
    onboardingpage2.cpp
    onboardingpage2.h
    onboardingpage2.ui
    onboardingpage3.cpp
    onboardingpage3.h
    onboardingpage3.ui
    onboardingpage4.cpp
    onboardingpage4.h
    onboardingpage4.ui
    onboardingpage5.cpp
    onboardingpage5.h
    onboardingpage5.ui
    onboarding_res.qrc
)

target_link_libraries(onboarding PUBLIC Qt::Widgets Qt::Concurrent libsync PersonalCloudResources customdialogs)

set_target_properties(onboarding PROPERTIES AUTOUIC ON AUTORCC ON)
target_include_directories(onboarding PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

app_apply_common_target_settings(onboarding)
