add_library(newwizard STATIC
    enums.cpp
    setupwidget.ui
    setupwidget.h
    setupwidget.cpp
    setupcontroller.h
    setupcontroller.cpp
    setupaccountbuilder.cpp
    setupaccountbuilder.h
    jobs/resolveurljobfactory.cpp
    jobs/resolveurljobfactory.h
    jobs/checkbasicauthjobfactory.cpp
    jobs/checkbasicauthjobfactory.h
    jobs/discoverwebfingerservicejobfactory.cpp
    jobs/discoverwebfingerservicejobfactory.h
    jobs/webfingeruserinfojobfactory.cpp
    jobs/webfingeruserinfojobfactory.h

    setupcontext.cpp
    pages/credentialspage.cpp
    pages/credentialspage.h
    pages/credentialspage.ui
    pages/emailpage.cpp
    pages/emailpage.h
    pages/emailpage.ui
    pages/waitpage.cpp
    pages/waitpage.h
    pages/waitpage.ui
    pages/finishedpage.cpp
    pages/finishedpage.h
    pages/finishedpage.ui

    codedialog.cpp
    codedialog.h
    codedialog.ui
    loginservices/mdnsclient.cpp
    loginservices/mdnsclient.h
    loginservices/serviceconnector.cpp
    loginservices/serviceconnector.h
    loginservices/devicetypes.cpp
    loginservices/devicetypes.h
    loginservices/loginservices_res.qrc
)

target_link_libraries(newwizard PUBLIC Qt::Widgets libsync curatorResources)

set_target_properties(newwizard PROPERTIES AUTOUIC ON AUTORCC ON)
target_include_directories(newwizard PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

apply_common_target_settings(newwizard)
