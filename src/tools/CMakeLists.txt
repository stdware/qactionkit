function(qak_install_tool _target)
    if(QACTIONKIT_INSTALL)
        if(QACTIONKIT_VCPKG_TOOLS_HINT)
            set(_tools_dir tools/${QACTIONKIT_INSTALL_NAME})
        else()
            set(_tools_dir ${CMAKE_INSTALL_BINDIR})
        endif()

        install(TARGETS ${_target}
            EXPORT ${QACTIONKIT_INSTALL_NAME}Targets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL
        )
    endif()
endfunction()

add_subdirectory(aec)