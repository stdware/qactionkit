project(QAKExample_MainWindow)

file(GLOB _src *.h *.cpp)

qm_find_qt(Core)
qak_add_action_extension(_mainwindow_action_src mainwindow-actions.xml)
qak_add_action_extension(_plugin_action_src plugin-actions.xml)

qak_add_example(${PROJECT_NAME}
    SOURCES ${_src} ${_mainwindow_action_src} ${_plugin_action_src}
    QT_LINKS Core Gui Widgets
    LINKS QAKWidgets
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED TRUE
)