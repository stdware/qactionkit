project(tst_ActionRegistry)

qm_find_qt(Core)

qak_add_action_extension(_action_src "actions.xml"
    FUNCTION testActions
    NAMESPACE qak::test
)

qak_add_auto_test(SOURCES ${_action_src})
