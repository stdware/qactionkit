function(qak_install_tool _target)
    if(QACTIONKIT_INSTALL)
        install(TARGETS ${_target}
            EXPORT ${QACTIONKIT_INSTALL_NAME}Targets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL
        )
    endif()
endfunction()

# add_subdirectory(aec)