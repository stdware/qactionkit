project(tst_ActionRegistry)

qm_find_qt(Core)

qak_add_action_extension(_action_src "actions.xml"
    FUNCTION testActions
    NAMESPACE qak::test
)

# Insertions that are skipped, whose warnings would otherwise appear in every test
qak_add_action_extension(_insertion_src "insertions.xml"
    FUNCTION testInsertions
    NAMESPACE qak::test
)

qak_add_auto_test(SOURCES ${_action_src} ${_insertion_src})
