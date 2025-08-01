#
# Asp top-level build specification.
#

cmake_minimum_required(VERSION 3.5)

configure_file(version.txt version.txt COPYONLY) # Force reread on change
file(STRINGS version.txt VERSION)

project(asp
    # The engine ABI version (used in compatibility checking) consists of the
    # major and minor components of the top-level version. Therefore, the
    # version of the engine (aspe) and compiler (aspc) projects must be
    # compatible with this version (major and minor components must match).
    VERSION ${VERSION}
    )

option(BUILD_FOR_HOST
    "Enable/disable building tools for the host environment"
    FALSE)
option(BUILD_FOR_TARGET
    "Enable/disable building for the script engine target environment"
    FALSE)
option(INSTALL_DEV
    "Enable/disable installation of development files (e.g., headers)"
    FALSE)

# If no build options are chosen, build them all.
if(NOT ${BUILD_FOR_HOST} AND NOT ${BUILD_FOR_TARGET})
    set(BUILD_FOR_HOST TRUE)
    set(BUILD_FOR_TARGET TRUE)
endif()

option(BUILD_SHARED_LIBS
    "Enable/disable building of shared libraries"
    FALSE)
option(BUILD_TEST_TARGETS
    "Enable/disable building of tests"
    FALSE)

# Debug option.
option(ENABLE_DEBUG
    "Enable/disable debug output for the engine and standalone application"
    FALSE)

# Test targets use some internal functions which are not made public when
# building shared libraries. Therefore, we must enforce static libraries when
# building the test targets.
if(BUILD_TEST_TARGETS)
    set(BUILD_SHARED_LIBS FALSE)
endif()

# System definitions.
if(WIN32)
    set(PATH_NAME_SEPARATOR [[;]])
    set(C_PATH_NAME_SEPARATOR [['\\;']])
    set(C_FILE_NAME_SEPARATORS [["\\/"]])
    set(C_COMMAND_OPTION_PREFIXES [["-/"]])
else()
    set(PATH_NAME_SEPARATOR :)
    set(C_PATH_NAME_SEPARATOR ':')
    set(C_FILE_NAME_SEPARATORS [["/"]])
    set(C_COMMAND_OPTION_PREFIXES [["-"]])
endif()

# Compile options.
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(
        -pedantic -Werror
        -Wno-parentheses -Wno-switch
        -Wwrite-strings -Wpointer-arith
        )
endif()
if(WIN32 AND MSVC)
    add_compile_options(/D_CRT_SECURE_NO_WARNINGS)
endif()
if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    # Unsupported options when using clang-cl
    add_compile_options(/Zc:preprocessor /wd4554)
endif()

add_subdirectory(engine)

if(BUILD_FOR_HOST)
    add_subdirectory(lemon)
    add_subdirectory(compiler)
    add_subdirectory(appspec)
    add_subdirectory(info)
    add_subdirectory(util)
endif()

if(BUILD_FOR_HOST AND BUILD_FOR_TARGET)
    add_subdirectory(standalone)
endif()

if(BUILD_TEST_TARGETS)
    add_subdirectory(test)
endif()

# Package specifications.
include(package.cmake)
