cmake_minimum_required(VERSION 3.19)

project(QActionKit VERSION 1.0.0.0 LANGUAGES CXX)

# ----------------------------------
# Build Options
# ----------------------------------
option(QACTIONKIT_BUILD_STATIC "Build static libraries" OFF)
option(QACTIONKIT_BUILD_WIDGETS "Build widgets module" ON)
option(QACTIONKIT_BUILD_QUICK "Build quick module" OFF)
option(QACTIONKIT_BUILD_EXAMPLES "Build examples" OFF)
option(QACTIONKIT_BUILD_DOCUMENTATIONS "Build documentations" OFF)
option(QACTIONKIT_INSTALL "Install library" ON)

option(QACTIONKIT_ENABLE_QT_WINDOW_CONTEXT "Enable Qt Window Context anyway" OFF)
option(QACTIONKIT_ENABLE_WINDOWS_SYSTEM_BORDERS "Enable system borders on Windows" ON)
option(QACTIONKIT_ENABLE_STYLE_AGENT "Enable building style agent" ON)


# ----------------------------------
# CMake Settings
# ----------------------------------
if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /manifest:no")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /manifest:no")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /manifest:no")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")

    if(NOT DEFINED CMAKE_DEBUG_POSTFIX)
        set(CMAKE_DEBUG_POSTFIX "d")
    endif()
elseif(MINGW)
    set(CMAKE_STATIC_LIBRARY_PREFIX "")
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif()

if(QACTIONKIT_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)
endif()

# ----------------------------------
# Project Variables
# ----------------------------------
set(QACTIONKIT_VERSION ${PROJECT_VERSION})
set(QACTIONKIT_INSTALL_NAME ${PROJECT_NAME})

# ----------------------------------
# Find basic dependencies
# ----------------------------------
# find_package(qmsetup QUIET)

# if(NOT TARGET qmsetup::library)
#     # Modify this variable according to your project structure
#     set(_source_dir ${CMAKE_CURRENT_SOURCE_DIR}/qmsetup)

#     # Import install function
#     include("${_source_dir}/cmake/modules/private/InstallPackage.cmake")

#     # Install package in place
#     set(_package_path)
#     qm_install_package(qmsetup
#         SOURCE_DIR ${_source_dir}
#         BUILD_TYPE Release
#         RESULT_PATH _package_path
#     )

#     # Find package again
#     find_package(qmsetup REQUIRED PATHS ${_package_path})

#     # Update import path
#     set(qmsetup_DIR ${_package_path} CACHE PATH "" FORCE)
# endif()
find_package(qmsetup REQUIRED)

qm_import(Filesystem)
qm_init_directories()

# ----------------------------------
# Add source modules
# ----------------------------------
add_subdirectory(src)

if(QACTIONKIT_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()