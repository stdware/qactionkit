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

# A plugin that declares items of actions.xml again
qak_add_action_extension(_plugin_src "plugin.xml"
    FUNCTION testPlugin
    NAMESPACE qak::test
)

# Two plugins that insert into the same positions with priorities
qak_add_action_extension(_priority_a_src "priority-a.xml"
    FUNCTION testPriorityA
    NAMESPACE qak::test
)
qak_add_action_extension(_priority_b_src "priority-b.xml"
    FUNCTION testPriorityB
    NAMESPACE qak::test
)

# Another extension with the id of priority-a.xml
qak_add_action_extension(_same_id_src "same-id.xml"
    FUNCTION testSameId
    NAMESPACE qak::test
)

qak_add_auto_test(SOURCES ${_action_src} ${_insertion_src} ${_plugin_src} ${_priority_a_src}
    ${_priority_b_src} ${_same_id_src})
