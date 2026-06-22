include(APPApplyCommonSettings)
find_package(Qt6 COMPONENTS Test REQUIRED)

function(personalcloud_add_test test_class)
    set(PERSONALCLOUD_TEST_CLASS ${test_class})
    string(TOLOWER "${PERSONALCLOUD_TEST_CLASS}" PERSONALCLOUD_TEST_CLASS_LOWERCASE)
    set(SRC_PATH test${PERSONALCLOUD_TEST_CLASS_LOWERCASE}.cpp)
    if (IS_DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR}/test${PERSONALCLOUD_TEST_CLASS_LOWERCASE}/)
        set(SRC_PATH test${PERSONALCLOUD_TEST_CLASS_LOWERCASE}/${SRC_PATH})
    endif()

    add_executable("${PERSONALCLOUD_TEST_CLASS}Test"
        ${SRC_PATH}
        ${ARGN}
    )
    set_target_properties("${PERSONALCLOUD_TEST_CLASS}Test" PROPERTIES AUTOMOC ON)
    target_link_libraries("${PERSONALCLOUD_TEST_CLASS}Test" PersonalCloudCore syncenginetestutils testutilsloader Qt::Test)
    add_test(NAME "${PERSONALCLOUD_TEST_CLASS}Test" COMMAND "${PERSONALCLOUD_TEST_CLASS}Test")
    app_apply_common_target_settings(${PERSONALCLOUD_TEST_CLASS}Test)
    target_compile_definitions(${PERSONALCLOUD_TEST_CLASS}Test PRIVATE SOURCEDIR="${PROJECT_SOURCE_DIR}" QT_FORCE_ASSERTS)

    target_include_directories(${PERSONALCLOUD_TEST_CLASS}Test PRIVATE "${CMAKE_SOURCE_DIR}/test/")
endfunction()
