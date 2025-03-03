if(NOT CK_CKAEC_EXECUTABLE)
    if(TARGET QActionKit::qak_aec)
        get_target_property(CK_CKAEC_EXECUTABLE QActionKit::qak_aec LOCATION)
    else()
        message(FATAL_ERROR "QActionKit: tool \"qak_aec\" not found!")
    endif()
endif()

#[[
Add an action extension generating target.

    qak_add_action_extension(<OUT> <manifest>
        [IDENTIFIER <identifier>]
        [DEFINES    <defines>...]
        [DEPENDS    <dependencies>...]
        [OPTIONS    <options>...]
    )
]] #
function(qak_add_action_extension _outfiles _manifest)
    set(options)
    set(oneValueArgs IDENTIFIER)
    set(multiValueArgs DEFINES DEPENDS OPTIONS)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # helper macro to set up a moc rule
    function(_create_command _infile _outfile _options _depends)
        # Pass the parameters in a file.  Set the working directory to
        # be that containing the parameters file and reference it by
        # just the file name.  This is necessary because the moc tool on
        # MinGW builds does not seem to handle spaces in the path to the
        # file given with the @ syntax.
        get_filename_component(_outfile_name "${_outfile}" NAME)
        get_filename_component(_outfile_dir "${_outfile}" PATH)

        if(_outfile_dir)
            set(_working_dir WORKING_DIRECTORY ${_outfile_dir})
        endif()

        set(_cmd ${QAK_AEC_EXECUTABLE} ${_options} -o "${_outfile}" "${_infile}")

        if(WIN32)
            # Add Qt Core to PATH
            get_target_property(_loc Qt${QT_VERSION_MAJOR}::Core IMPORTED_LOCATION_RELEASE)
            get_filename_component(_dir ${_loc} DIRECTORY)
            set(_cmd COMMAND set "Path=${_dir}\;%Path%\;" COMMAND ${_cmd})
        else()
            set(_cmd COMMAND ${_cmd})
        endif()

        add_custom_command(OUTPUT ${_outfile}
            ${_cmd}
            DEPENDS ${_infile} ${_depends}
            ${_working_dir}
            VERBATIM
        )
    endfunction()

    set(_options)

    if(FUNC_IDENTIFIER)
        list(APPEND _options -i ${FUNC_IDENTIFIER})
    endif()

    if(FUNC_DEFINES)
        foreach(_item IN LISTS FUNC_DEFINES)
            list(APPEND _options -D${_item})
        endforeach()
    endif()

    list(APPEND _options ${FUNC_OPTIONS})

    set(_outfile)
    get_filename_component(_manifest ${_manifest} ABSOLUTE)
    qm_make_output_file(${_manifest} ckaec_ cpp _outfile)

    # Create command
    _create_command(${_manifest} ${_outfile} "${_options}" "${FUNC_DEPENDS}")

    set(${_outfiles} ${_outfile} PARENT_SCOPE)
endfunction()