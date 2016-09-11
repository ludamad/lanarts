set(VORBIS_SRC
    lib/analysis.c
    lib/bitrate.c
    lib/block.c
    lib/codebook.c
    lib/envelope.c
    lib/floor0.c
    lib/floor1.c
    lib/info.c
    lib/lookup.c
    lib/lpc.c
    lib/lsp.c
    lib/mapping0.c
    lib/mdct.c
    lib/psy.c
    lib/registry.c
    lib/res0.c
    lib/sharedbook.c
    lib/smallft.c
    lib/synthesis.c
    lib/vorbisenc.c
    lib/window.c
)

add_library(VORBIS SHARED
	${VORBIS_SRC}
)

set_target_properties(VORBIS
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_link_libraries (VORBIS
	OGG
)

if(WIN32)
	if(MSVC)
		configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/win32/vorbis.def ${CMAKE_CURRENT_BINARY_DIR}/win32/vorbis.def @ONLY)
		set_target_properties(VORBIS PROPERTIES LINK_FLAGS "/DEF:win32/vorbis.def")
	endif()
endif()
set_target_properties(VORBIS
	PROPERTIES PREFIX "" OUTPUT_NAME "libvorbis"
)
