project(tst_aec)

qak_add_auto_test()

target_link_libraries(${PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:qmxmladaptor>)