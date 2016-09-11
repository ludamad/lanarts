cmake_minimum_required (VERSION 2.8)

project (MODPLUG)

include(CheckLibraryExists)
include(CheckIncludeFiles)
include(CheckIncludeFile)

set(MODPLUG_SRC
	src/fastmix.cpp
	src/load_669.cpp
	src/load_abc.cpp
	src/load_amf.cpp
	src/load_ams.cpp
	src/load_dbm.cpp
	src/load_dmf.cpp
	src/load_dsm.cpp
	src/load_far.cpp
	src/load_it.cpp
	src/load_j2b.cpp
	src/load_mdl.cpp
	src/load_med.cpp
	src/load_mid.cpp
	src/load_mod.cpp
	src/load_mt2.cpp
	src/load_mtm.cpp
	src/load_okt.cpp
	src/load_pat.cpp
	src/load_psm.cpp
	src/load_ptm.cpp
	src/load_s3m.cpp
	src/load_stm.cpp
	src/load_ult.cpp
	src/load_umx.cpp
	src/load_wav.cpp
	src/load_xm.cpp
	src/mmcmp.cpp
	src/modplug.cpp
	src/snd_dsp.cpp
	src/snd_flt.cpp
	src/snd_fx.cpp
	src/sndfile.cpp
	src/sndmix.cpp
)

set(MODPLUG_SOURCES
	${MODPLUG_SRC}
)

set(MODPLUG_NO_FILESAVE OFF CACHE BOOL "Disable file saving support")
set(MODPLUG_NEWMIKMOD OFF CACHE BOOL "Use a new mikmod")

if (MODPLUG_NO_FILESAVE)
	add_definitions(-DMODPLUG_NO_FILESAVE)
endif()
if (MODPLUG_NEWMIKMOD)
	if (NOT TARGET MIKMOD)
		add_subdirectory(../libmikmod-3.1.12 "${CMAKE_CURRENT_BINARY_DIR}/libmikmod-3.1.12")
	endif()
	include_directories(../libmikmod-3.1.12/include)
	add_definitions(-DNEWMIKMOD)
endif()

add_library(MODPLUG SHARED
	${MODPLUG_SOURCES}
)

set_target_properties(MODPLUG
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(WIN32)
	if(MSVC)
		configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/src/libmodplug.def ${CMAKE_CURRENT_BINARY_DIR}/src/libmodplug.def @ONLY)
		set_target_properties(MODPLUG PROPERTIES LINK_FLAGS "/DEF:src/libmodplug.def")
	endif()
endif()
include_directories("${MODPLUG_SOURCE_DIR}/src")
include_directories("${MODPLUG_SOURCE_DIR}/src/libmodplug")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY build)
set_target_properties(MODPLUG
	PROPERTIES PREFIX "" OUTPUT_NAME "libmodplug"
)
