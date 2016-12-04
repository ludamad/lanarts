cmake_minimum_required (VERSION 2.8)
project (SDLMIXER)

list(APPEND CMAKE_MODULE_PATH "${SDLMIXER_SOURCE_DIR}/cmake")

include(CheckSymbolExists)
include(CheckLibraryExists)
include(CheckIncludeFiles)
include(CheckIncludeFile)

include_directories(../sdl2/include)
set(SDLMIXER_LIBRARIES "")

add_definitions(-DMID_MUSIC)
add_definitions(-DWAV_MUSIC)
add_definitions(-DUSE_TIMIDITY_MIDI)
include_directories("${SDLMIXER_SOURCE_DIR}/timidity")
set(SDLMIXER_TIMIDITY_SRC
        timidity/common.c
        timidity/ctrlmode.c
        timidity/filter.c
        timidity/instrum.c
        timidity/mix.c
        timidity/output.c
        timidity/playmidi.c
        timidity/readmidi.c
        timidity/resample.c
        timidity/sdl_a.c
        timidity/sdl_c.c
        timidity/tables.c
        timidity/timidity.c
)

list(APPEND SDLMIXER_ADDITIONAL_SRC ${SDLMIXER_TIMIDITY_SRC})
add_definitions(-DOGG_MUSIC)
#if (NOT TARGET OGG)
        add_subdirectory(external/libogg-1.3.1)
#endif()
include_directories(external/libogg-1.3.1/include)

add_subdirectory(external/libvorbis-1.3.3)
include_directories(external/libvorbis-1.3.3/include)
set(SDLMIXER_SRC
	dynamic_flac.c
	dynamic_mod.c
	dynamic_modplug.c
	dynamic_mp3.c
	dynamic_ogg.c
	effect_position.c
	effect_stereoreverse.c
	effects_internal.c
	load_aiff.c
	load_flac.c
	load_mp3.c
	load_ogg.c
	load_voc.c
	mixer.c
	music.c
	music_cmd.c
	music_flac.c
	music_mad.c
	music_mod.c
	music_modplug.c
	music_ogg.c
	wavestream.c
)

set(SDLMIXER_SOURCES
	${SDLMIXER_SRC} ${SDLMIXER_ADDITIONAL_SRC}
)

include_directories(./)

add_library(SDL2-mixer STATIC
	${SDLMIXER_SOURCES}
)

target_link_libraries (SDL2-mixer SDL2-static OGG vorbis)

add_dependencies(SDL2-mixer SDL2-static OGG vorbis)
