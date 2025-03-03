project(QAKExample_MainWindow)

file(GLOB _src *.h *.cpp)

qm_find_qt(Core)
qak_add_action_extension(_core_action_src ../shared/core-actions.xml)
qak_add_action_extension(_plugin_action_src ../shared/plugin-actions.xml)

qak_add_example(${PROJECT_NAME}
    SOURCES ${_src} ${_core_action_src} ${_plugin_action_src}
    FEATURES cxx_std_17
    QT_LINKS Core Gui Widgets
    LINKS QAKWidgets
)