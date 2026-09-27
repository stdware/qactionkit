if(NOT QAK_AEC_EXECUTABLE)
    if(TARGET QActionKit::qak_aec)
        get_target_property(QAK_AEC_EXECUTABLE QActionKit::qak_aec LOCATION)
    else()
        message(FATAL_ERROR "QActionKit: tool \"qak_aec\" not found!")
    endif()
endif()

#[[
    Create the names of output files preserving relative dirs. (Ported from MOC command)

    qak_make_output_file(<infile> <prefix> <ext> <OUT>)

    OUT: output source file paths
#]]
function(qak_make_output_file _infile _prefix _ext _out)
    string(LENGTH ${CMAKE_CURRENT_BINARY_DIR} _binlength)
    string(LENGTH ${_infile} _infileLength)
    set(_checkinfile ${CMAKE_CURRENT_SOURCE_DIR})

    if(_infileLength GREATER _binlength)
        string(SUBSTRING "${_infile}" 0 ${_binlength} _checkinfile)

        if(_checkinfile STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
            file(RELATIVE_PATH _rel ${CMAKE_CURRENT_BINARY_DIR} ${_infile})
        else()
            file(RELATIVE_PATH _rel ${CMAKE_CURRENT_SOURCE_DIR} ${_infile})
        endif()
    else()
        file(RELATIVE_PATH _rel ${CMAKE_CURRENT_SOURCE_DIR} ${_infile})
    endif()

    if(CMAKE_HOST_WIN32 AND _rel MATCHES "^([a-zA-Z]):(.*)$") # absolute path
        set(_rel "${CMAKE_MATCH_1}_${CMAKE_MATCH_2}")
    endif()

    set(_outfile "${CMAKE_CURRENT_BINARY_DIR}/${_rel}")
    string(REPLACE ".." "__" _outfile ${_outfile})
    get_filename_component(_outpath ${_outfile} PATH)
    get_filename_component(_outfile ${_outfile} NAME_WLE)

    file(MAKE_DIRECTORY ${_outpath})
    set(${_out} ${_outpath}/${_prefix}${_outfile}.${_ext} PARENT_SCOPE)
endfunction()

#[[
Compile an action extension manifest with AEC.

    qak_add_action_extension(<OUT> <manifest>
        FUNCTION           <name>
        [NAMESPACE         <namespace>]
        [EXPORT_DIRECTIVE  <macro>]
        [EXPORT_FILE_NAME  <header>]
        [DEFINES           <defines>...]
        [DEPENDS           <dependencies>...]
        [OPTIONS           <options>...]
    )

    OUT: the generated source file and header, to be added to a target.

    The header, named <manifest base name>.qak.h, is generated in CMAKE_CURRENT_BINARY_DIR and
    declares "const QAK::ActionExtension *<name>()" in <namespace>. The target that includes the
    header needs CMAKE_CURRENT_BINARY_DIR in its include directories. EXPORT_DIRECTIVE is placed
    before the declaration, and EXPORT_FILE_NAME is the header that defines it.
]] #
function(qak_add_action_extension _outfiles _manifest)
    set(options)
    set(oneValueArgs FUNCTION NAMESPACE EXPORT_DIRECTIVE EXPORT_FILE_NAME)
    set(multiValueArgs DEFINES DEPENDS OPTIONS)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT TARGET Qt${QT_VERSION_MAJOR}::Core)
        message(FATAL_ERROR "qak_add_action_extension: qt library not defined. Add find_package(Qt5 COMPONENTS Core) to CMake to enable.")
    endif()

    if(NOT FUNC_FUNCTION)
        message(FATAL_ERROR "qak_add_action_extension: FUNCTION is not specified for ${_manifest}.")
    endif()

    if(FUNC_EXPORT_FILE_NAME AND NOT FUNC_EXPORT_DIRECTIVE)
        message(FATAL_ERROR "qak_add_action_extension: EXPORT_FILE_NAME is given without EXPORT_DIRECTIVE for ${_manifest}.")
    endif()

    # helper macro to set up a moc rule
    function(_qak_create_command _infile _outfile _header _options _depends)
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

        set(_cmd ${QAK_AEC_EXECUTABLE} ${_options} -o "${_outfile}" --header "${_header}" "${_infile}")

        if(WIN32)
            # Add Qt Core to PATH
            get_target_property(_loc Qt${QT_VERSION_MAJOR}::Core IMPORTED_LOCATION_RELEASE)
            get_filename_component(_dir ${_loc} DIRECTORY)
            set(_cmd COMMAND set "Path=${_dir}\;%Path%\;" COMMAND ${_cmd})
        else()
            set(_cmd COMMAND ${_cmd})
        endif()

        # The command depends on AEC as a file, so that a rebuilt AEC regenerates the output. A
        # generator expression in COMMAND adds only a dependency on the target, which builds AEC
        # first without running the command again.
        add_custom_command(OUTPUT ${_outfile} ${_header}
            ${_cmd}
            DEPENDS ${_infile} ${QAK_AEC_EXECUTABLE} ${_depends}
            ${_working_dir}
            VERBATIM
        )
    endfunction()

    set(_options --function ${FUNC_FUNCTION})

    if(FUNC_NAMESPACE)
        list(APPEND _options --namespace ${FUNC_NAMESPACE})
    endif()

    if(FUNC_EXPORT_DIRECTIVE)
        list(APPEND _options --export-directive ${FUNC_EXPORT_DIRECTIVE})
    endif()

    if(FUNC_EXPORT_FILE_NAME)
        list(APPEND _options --export-file-name ${FUNC_EXPORT_FILE_NAME})
    endif()

    if(FUNC_DEFINES)
        foreach(_item IN LISTS FUNC_DEFINES)
            list(APPEND _options -D${_item})
        endforeach()
    endif()

    list(APPEND _options ${FUNC_OPTIONS})

    set(_outfile)
    get_filename_component(_manifest ${_manifest} ABSOLUTE)
    qak_make_output_file(${_manifest} qak_ cpp _outfile)

    get_filename_component(_manifest_name ${_manifest} NAME_WLE)
    set(_header "${CMAKE_CURRENT_BINARY_DIR}/${_manifest_name}.qak.h")

    # Create command
    _qak_create_command(${_manifest} ${_outfile} ${_header} "${_options}" "${FUNC_DEPENDS}")

    set(${_outfiles} ${_outfile} ${_header} PARENT_SCOPE)
endfunction()