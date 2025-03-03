qm_import(Preprocess)

string(TIMESTAMP _current_year "%Y")

set(QACTIONKIT_GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/../include)
set(QACTIONKIT_BUILD_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/../etc/include)

set(QACTIONKIT_ENABLED_TARGETS)
set(QACTIONKIT_ENABLED_SUBDIRECTORIES)

# ----------------------------------
# Configurations
# ----------------------------------
set(QMSETUP_DEFINITION_SCOPE DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(QMSETUP_DEFINITION_NUMERICAL on)

qm_generate_config(${QACTIONKIT_BUILD_INCLUDE_DIR}/QAKCore/qakconfig.h)

if(QACTIONKIT_INSTALL)
    install(FILES ${QACTIONKIT_BUILD_INCLUDE_DIR}/QAKCore/qakconfig.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${QACTIONKIT_INSTALL_NAME}/QAKCore
    )
endif()

# ----------------------------------
# CMake API
# ----------------------------------
macro(qak_add_library _target)
    set(options AUTOGEN NO_SYNC_INCLUDE NO_WIN_RC)
    set(oneValueArgs SYNC_INCLUDE_PREFIX PREFIX)
    set(multiValueArgs SYNC_INCLUDE_OPTIONS)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(FUNC_AUTOGEN)
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTOUIC ON)
        set(CMAKE_AUTORCC ON)
    endif()

    if(QACTIONKIT_BUILD_STATIC)
        set(_type STATIC)
    else()
        set(_type SHARED)
    endif()

    add_library(${_target} ${_type})

    if(WIN32 AND NOT FUNC_NO_WIN_RC AND(${_type} STREQUAL "SHARED"))
        qm_add_win_rc(${_target}
            NAME ${QACTIONKIT_INSTALL_NAME}
            DESCRIPTION ${QACTIONKIT_DESCRIPTION}
            COPYRIGHT ${QACTIONKIT_COPYRIGHT}
        )
    endif()

    if(FUNC_PREFIX)
        set(_prefix_option PREFIX ${FUNC_PREFIX})
    else()
        set(_prefix_option)
    endif()

    # Set global definitions
    qm_export_defines(${_target} ${_prefix_option})

    # Configure target
    qm_configure_target(${_target} ${FUNC_UNPARSED_ARGUMENTS})

    # Add include directories
    target_include_directories(${_target} PRIVATE ${QACTIONKIT_BUILD_INCLUDE_DIR})
    target_include_directories(${_target} PRIVATE .)

    # Library name
    if(${_target} MATCHES "^QAK(.+)")
        set(_name ${CMAKE_MATCH_1})
        set_target_properties(${_target} PROPERTIES EXPORT_NAME ${_name})
    else()
        set(_name ${_target})
    endif()

    add_library(${QACTIONKIT_INSTALL_NAME}::${_name} ALIAS ${_target})

    if(FUNC_SYNC_INCLUDE_PREFIX)
        set(_inc_name ${FUNC_SYNC_INCLUDE_PREFIX})
    else()
        set(_inc_name ${_target})
    endif()

    set(_install_options)

    if(QACTIONKIT_INSTALL)
        install(TARGETS ${_target}
            EXPORT ${QACTIONKIT_INSTALL_NAME}Targets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
        )

        target_include_directories(${_target} PUBLIC
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${QACTIONKIT_INSTALL_NAME}>"
        )

        set(_install_options
            INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}/${QACTIONKIT_INSTALL_NAME}/${_inc_name}"
        )
    endif()

    if(NOT FUNC_NO_SYNC_INCLUDE)
        # Generate a standard include directory in build directory
        qm_sync_include(. "${QACTIONKIT_GENERATED_INCLUDE_DIR}/${_inc_name}" ${_install_options}
            ${FUNC_SYNC_INCLUDE_OPTIONS} FORCE
        )
        target_include_directories(${_target} PUBLIC
            "$<BUILD_INTERFACE:${QACTIONKIT_GENERATED_INCLUDE_DIR}>"
        )
    endif()
endmacro()

# ----------------------------------
# Main Project
# ----------------------------------
add_subdirectory(3rdparty)

add_subdirectory(core)

add_subdirectory(tools)

if(QACTIONKIT_BUILD_WIDGETS)
    add_subdirectory(widgets)
endif()

if(QACTIONKIT_BUILD_QUICK)
    add_subdirectory(quick)
endif()

# ----------------------------------
# Documentation
# ----------------------------------
if(QACTIONKIT_BUILD_DOCUMENTATIONS)
    if(NOT DOXYGEN_EXECUTABLE)
        find_package(Doxygen REQUIRED)
    endif()

    set(_install_options)

    if(QACTIONKIT_INSTALL)
        set(_install_options INSTALL_DIR share/doc/${QACTIONKIT_INSTALL_NAME})
    endif()

    set(_doc_targets ${QACTIONKIT_ENABLED_TARGETS})

    set(QACTIONKIT_DOXYGEN_TAGFILE
        ${CMAKE_BUILD_SHARE_DIR}/doc/${QACTIONKIT_INSTALL_NAME}/${QACTIONKIT_INSTALL_NAME}_tagfile.xml
        CACHE FILEPATH "QActionKit doxygen tag file path" FORCE
    )

    qm_import(Doxygen)
    qm_setup_doxygen(QActionKit_RunDoxygen
        NAME "QActionKit"
        DESCRIPTION "${QACTIONKIT_DESCRIPTION}"
        MDFILE ../README.md
        OUTPUT_DIR ${CMAKE_BUILD_SHARE_DIR}/doc/${QACTIONKIT_INSTALL_NAME}
        INPUT ${QACTIONKIT_ENABLED_SUBDIRECTORIES}
        TARGETS ${_doc_targets}
        DEPENDS ${_doc_targets}
        NO_EXPAND_MACROS Q_OBJECT Q_GADGET Q_DECLARE_TR_FUNCTIONS
        COMPILE_DEFINITIONS Q_SIGNALS=Q_SIGNALS Q_SLOTS=Q_SLOTS
        GENERATE_TAGFILE "${QACTIONKIT_DOXYGEN_TAGFILE}"
        ${_install_options}
    )
endif()

# ----------------------------------
# Install
# ----------------------------------
if(QACTIONKIT_INSTALL)
    set(_install_dir ${CMAKE_INSTALL_LIBDIR}/cmake/${QACTIONKIT_INSTALL_NAME})
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/QActionKitMacros.cmake"
        DESTINATION ${_install_dir}
    )
    qm_basic_install(
        NAME ${QACTIONKIT_INSTALL_NAME}
        VERSION ${QACTIONKIT_VERSION}
        INSTALL_DIR ${_install_dir}
        CONFIG_TEMPLATE "${QACTIONKIT_INSTALL_NAME}Config.cmake.in"
        NAMESPACE ${QACTIONKIT_INSTALL_NAME}::
        EXPORT ${QACTIONKIT_INSTALL_NAME}Targets
        WRITE_CONFIG_OPTIONS NO_CHECK_REQUIRED_COMPONENTS_MACRO
    )
endif()