set(QAK_AEC_EXECUTABLE "$<TARGET_FILE:qak_aec>")
include("${CMAKE_CURRENT_LIST_DIR}/../src/QActionKitMacros.cmake")

macro(qak_add_example _target)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)

    add_executable(${_target})
    qm_configure_target(${_target} ${ARGN})
endmacro()

add_subdirectory(mainwindow)