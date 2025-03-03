project(qak_aec
    VERSION ${QACTIONKIT_VERSION}
    LANGUAGES CXX
)

add_executable(${PROJECT_NAME})

file(GLOB _src *.h *.cpp)
qm_configure_target(${PROJECT_NAME}
    SOURCES ${_src}
    LINKS $<BUILD_INTERFACE:stdcorelib> $<BUILD_INTERFACE:qmxmladaptor> $<BUILD_INTERFACE:util> QAKCore
    QT_LINKS Core
    DEFINES APP_VERSION="${PROJECT_VERSION}"
    FEATURES cxx_std_17
)

qm_add_win_rc(${PROJECT_NAME}
    NAME ${CHORUSKIT_INSTALL_NAME}
    DESCRIPTION "QActionKit Action Extension Compiler"
    COPYRIGHT ${QACTIONKIT_COPYRIGHT}
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE
)

qak_install_tool(${PROJECT_NAME})