project(QAKExample_MainWindow)

file(GLOB _src *.h *.cpp)

qm_find_qt(Core)

# Create action extensions and add generated source files to the target
qak_add_action_extension(
    _core_action_src
    "../shared/core-actions.xml"
)
qak_add_action_extension(
    _plugin_action_src
    "../shared/plugin-actions.xml"
)
list(APPEND _src ${_core_action_src} ${_plugin_action_src})

qak_add_example(${PROJECT_NAME}
    SOURCES ${_src}
    FEATURES cxx_std_17
    QT_LINKS Core Gui Widgets
    LINKS QAKWidgets
)