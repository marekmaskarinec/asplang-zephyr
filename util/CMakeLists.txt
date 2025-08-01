#
# Asp info utility build specification.
#

cmake_minimum_required(VERSION 3.5)

set(PARENT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})

configure_file(version.txt version.txt COPYONLY) # Force reread on change
file(STRINGS version.txt VERSION)

project(aspinfo VERSION ${VERSION})

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

add_executable(aspinfo
    main.cpp
    )

set_property(TARGET aspinfo
    PROPERTY VERSION ${ABI_VERSION}
    )

target_compile_definitions(aspinfo PRIVATE
    ASP_INFO_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    ASP_INFO_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    ASP_INFO_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    ASP_INFO_VERSION_TWEAK=${PROJECT_VERSION_TWEAK}
    $<$<NOT:$<STREQUAL:${C_COMMAND_OPTION_PREFIXES},>>:
        COMMAND_OPTION_PREFIXES=${C_COMMAND_OPTION_PREFIXES}>
    )

target_link_libraries(aspinfo
    aspd
    )

install(TARGETS aspinfo
    DESTINATION bin
    COMPONENT Runtime
    )
