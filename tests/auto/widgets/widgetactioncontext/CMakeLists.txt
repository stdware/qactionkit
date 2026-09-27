project(tst_WidgetActionContext)

qm_find_qt(Core Widgets)

qak_add_action_extension(_action_src "actions.xml"
    FUNCTION testActions
)

qak_add_auto_test(
    SOURCES ${_action_src}
    LINKS QAKWidgets
    QT_LINKS Widgets
)
