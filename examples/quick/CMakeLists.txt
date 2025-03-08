project(QAKExample_Quick)

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
    QT_LINKS Core Gui Qml Quick
    LINKS QAKQuick
)

if(QT_KNOWN_POLICY_QTP0001)
    qt_policy(SET QTP0001 NEW)
endif()

qt_add_qml_module(${PROJECT_NAME}
    URI QActionKit.Examples.Quick
    QML_FILES main.qml actions.qml
)