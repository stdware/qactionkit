project(tst_QuickActionContext)

qm_find_qt(Core Gui Qml Quick)

qak_add_action_extension(_action_src "actions.xml"
    FUNCTION testActions
)

qak_add_auto_test(
    SOURCES ${_action_src}
    LINKS QAKQuick
    QT_LINKS Gui Qml Quick
)

# The QML module of QActionKit is imported from the build directory, which is not an import path by
# default, and its plugin is not a dependency of the library that the test links.
target_compile_definitions(${PROJECT_NAME} PRIVATE
    QAK_TEST_QML_IMPORT_PATH="${QMSETUP_BUILD_DIR}/qml"
)
add_dependencies(${PROJECT_NAME} QAKQuickplugin)
