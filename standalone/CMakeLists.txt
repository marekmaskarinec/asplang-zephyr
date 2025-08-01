#
# Standalone Asp build specification.
#

cmake_minimum_required(VERSION 3.5)

set(PARENT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})

configure_file(version.txt version.txt COPYONLY) # Force reread on change
file(STRINGS version.txt VERSION)

project(asps VERSION ${VERSION})

set(ABI_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
if(NOT ${ABI_VERSION} VERSION_EQUAL ${PARENT_VERSION})
    message(FATAL_ERROR
        "${PROJECT_NAME} version not compatible with project version"
        )
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

add_custom_command(
    OUTPUT
        "${PROJECT_BINARY_DIR}/standalone.aspec"
        "${PROJECT_BINARY_DIR}/standalone.c"
        "${PROJECT_BINARY_DIR}/standalone.h"
    DEPENDS
        aspg
        "${PROJECT_SOURCE_DIR}/standalone.asps"
        "${aspe_SOURCE_DIR}/sys.asps"
        "${aspe_SOURCE_DIR}/type.asps"
        "${aspe_SOURCE_DIR}/collect.asps"
        "${aspe_SOURCE_DIR}/iter.asps"
        "${aspe_SOURCE_DIR}/math.asps"
    COMMAND
        ${CMAKE_COMMAND} -E env
        "ASP_SPEC_INCLUDE=${PATH_NAME_SEPARATOR}${aspe_SOURCE_DIR}"
        "$<TARGET_FILE:aspg>" "-q"
        "${PROJECT_SOURCE_DIR}/standalone.asps"
    )

add_executable(asps
    main.cpp
    standalone.c
    functions-print.cpp
    functions-sleep.cpp
    )

if(ENABLE_DEBUG)
    set_property(TARGET asps
        PROPERTY OUTPUT_NAME asps-d
        )
endif()
set_property(TARGET asps
    PROPERTY VERSION ${ABI_VERSION}
    )

target_compile_definitions(asps PRIVATE
    $<$<BOOL:${ENABLE_DEBUG}>:ASP_DEBUG>
    ASP_STANDALONE_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    ASP_STANDALONE_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    ASP_STANDALONE_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    ASP_STANDALONE_VERSION_TWEAK=${PROJECT_VERSION_TWEAK}
    $<$<NOT:$<STREQUAL:${C_COMMAND_OPTION_PREFIXES},>>:
        COMMAND_OPTION_PREFIXES=${C_COMMAND_OPTION_PREFIXES}>
    )

target_include_directories(asps PRIVATE
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}"
    )

target_link_libraries(asps
    aspe
    aspm
    aspd
    )

install(TARGETS asps
    DESTINATION bin
    COMPONENT Runtime
    )

if(WIN32)

    install(FILES "${PROJECT_BINARY_DIR}/standalone.aspec"
        DESTINATION etc/asp
        COMPONENT Runtime
        )

else()

    install(FILES "${PROJECT_BINARY_DIR}/standalone.aspec"
        DESTINATION etc/asp
        RENAME standalone-${ABI_VERSION}.aspec
        COMPONENT Runtime
        )

    install(
        CODE
            "execute_process(
            COMMAND ${CMAKE_COMMAND} -E create_symlink
            \"standalone-${ABI_VERSION}.aspec\"
            \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/etc/asp/standalone.aspec\"
            )"
        COMPONENT Runtime
        )

endif()
