function(qak_add_auto_test)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)
    add_executable(${PROJECT_NAME} main.cpp)
    qm_configure_target(${PROJECT_NAME}
        LINKS QAKCore
        QT_LINKS Test
    )
endfunction()

add_subdirectory(core)

add_subdirectory(tools)