set(VORBIS_FILE_SRC
	lib/vorbisfile.c
)

add_library(VORBIS_FILE SHARED
	${VORBIS_FILE_SRC}
)

set_target_properties(VORBIS_FILE
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_link_libraries (VORBIS_FILE
	VORBIS
	OGG
)

if(WIN32)
	if(MSVC)
		configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/win32/vorbisfile.def ${CMAKE_CURRENT_BINARY_DIR}/win32/vorbisfile.def @ONLY)
		set_target_properties(VORBIS_FILE PROPERTIES LINK_FLAGS "/DEF:win32/vorbisfile.def")
	endif()
endif()
set_target_properties(VORBIS_FILE
	PROPERTIES PREFIX "" OUTPUT_NAME "libvorbisfile"
)
