cmake_minimum_required (VERSION 2.8)
# enable_language(ASM-NASM)

project (OGG)

include(CheckLibraryExists)
include(CheckIncludeFiles)
include(CheckIncludeFile)

set(OGG_SRC
	src/bitwise.c
	src/framing.c
)

set(OGG_SOURCES "${OGG_SRC}")

include_directories("${OGG_SOURCE_DIR}/include")

add_library(OGG SHARED
	${OGG_SOURCES}
)

set_target_properties(OGG
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(WIN32)
	if(MSVC)
		configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/win32/ogg.def ${CMAKE_CURRENT_BINARY_DIR}/win32/ogg.def @ONLY)
		set_target_properties(OGG PROPERTIES LINK_FLAGS "/DEF:win32/ogg.def")
	endif()
endif()


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY build)
set_target_properties(OGG
	PROPERTIES PREFIX "" OUTPUT_NAME "libogg"
)
