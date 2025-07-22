
if (NOT CONFIG_ASPLANG_LIB)
	return()
endif()

zephyr_library()

configure_file(../engine/version.txt version.txt COPYONLY) # Force reread on change
file(STRINGS ${CMAKE_CURRENT_BINARY_DIR}/version.txt VERSION)

project(aspe
    VERSION ${VERSION}
    LANGUAGES C
    )

math(EXPR VERSION_VALUE
    "(${aspe_VERSION_MAJOR} << 24) +
     (${aspe_VERSION_MINOR} << 16) +
     (${aspe_VERSION_PATCH} << 8) +
     ${aspe_VERSION_TWEAK}"
    )

configure_file(../engine/asp-ver-template.h asp-ver.h)

zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../engine)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../info)
zephyr_include_directories(${CMAKE_CURRENT_BINARY_DIR})

zephyr_library_compile_options(-Wno-parentheses -Wno-switch)
zephyr_library_compile_definitions(
	ASP_ENGINE_VERSION_MAJOR=${aspe_VERSION_MAJOR}
	ASP_ENGINE_VERSION_MINOR=${aspe_VERSION_MINOR}
	ASP_ENGINE_VERSION_PATCH=${aspe_VERSION_PATCH}
	ASP_ENGINE_VERSION_TWEAK=${aspe_VERSION_TWEAK}
)
zephyr_library_sources(
	../engine/engine.c
	../engine/step.c
	../engine/bits.c
	../engine/api.c
	../engine/code.c
	../engine/data.c
	../engine/ref.c
	../engine/range.c
	../engine/stack.c
	../engine/sequence.c
	../engine/tree.c
	../engine/iterator.c
	../engine/assign.c
	../engine/compare.c
	../engine/operation.c
	../engine/function.c
	../engine/arguments.c
	../engine/lib-sys.c
	../engine/lib-type.c
	../engine/lib-collect.c
	../engine/lib-iter.c
	../engine/integer.c
	../engine/integer-result.c
)
