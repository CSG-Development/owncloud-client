include(OCApplyCommonSettings)
find_package(Qt6 COMPONENTS Test REQUIRED)

include(ECMAddTests)

function(curator_add_test test_class)
    set(CURATOR_TEST_CLASS ${test_class})
    string(TOLOWER "${CURATOR_TEST_CLASS}" CURATOR_TEST_CLASS_LOWERCASE)
    set(SRC_PATH test${CURATOR_TEST_CLASS_LOWERCASE}.cpp)
    if (IS_DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR}/test${CURATOR_TEST_CLASS_LOWERCASE}/)
        set(SRC_PATH test${CURATOR_TEST_CLASS_LOWERCASE}/${SRC_PATH})
    endif()

    ecm_add_test(${SRC_PATH}
        ${ARGN}
        TEST_NAME "${CURATOR_TEST_CLASS}Test"
        LINK_LIBRARIES
        curatorCore syncenginetestutils testutilsloader Qt::Test
    )
    apply_common_target_settings(${CURATOR_TEST_CLASS}Test)
    target_compile_definitions(${CURATOR_TEST_CLASS}Test PRIVATE SOURCEDIR="${PROJECT_SOURCE_DIR}" QT_FORCE_ASSERTS)

    target_include_directories(${CURATOR_TEST_CLASS}Test PRIVATE "${CMAKE_SOURCE_DIR}/test/")
    if (UNIX AND NOT APPLE)
        set_property(TEST ${CURATOR_TEST_CLASS}Test PROPERTY ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
    endif()
endfunction()
