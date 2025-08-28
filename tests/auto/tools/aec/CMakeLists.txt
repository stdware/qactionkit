project(tst_aec)

qak_add_auto_test()

target_sources(${PROJECT_NAME} PRIVATE "res.qrc")

target_link_libraries(${PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:qmxmladaptor>)