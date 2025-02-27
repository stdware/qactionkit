project(QAKCore
    VERSION ${QACTIONKIT_VERSION}
    LANGUAGES CXX
)

set(_src
    qakglobal.h
    action.h
    action_p.h
    action.cpp
    actionextension.h
    actionextension_p.h
    actionextension.cpp
    actionscope.h
    actionscope_p.h
    actionscope.cpp
)

qak_add_library(${PROJECT_NAME} AUTOGEN
    SOURCES ${_src}
    FEATURES cxx_std_17
    QT_LINKS Core Gui
    INCLUDE_PRIVATE *
    PREFIX QAK_CORE
)

set(QACTIONKIT_ENABLED_TARGETS ${QACTIONKIT_ENABLED_TARGETS} ${PROJECT_NAME} PARENT_SCOPE)
set(QACTIONKIT_ENABLED_SUBDIRECTORIES ${QACTIONKIT_ENABLED_SUBDIRECTORIES} core PARENT_SCOPE)