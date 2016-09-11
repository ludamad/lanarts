cmake_minimum_required (VERSION 2.8)
# enable_language(ASM-NASM)

project (FLAC)

include(CheckLibraryExists)
include(CheckIncludeFiles)
include(CheckIncludeFile)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/nasm.cmake)

# Ogg Vorbis

if (NOT TARGET OGG)
	add_subdirectory(../libogg-1.3.1 "${CMAKE_CURRENT_BINARY_DIR}/libogg")
endif()
include_directories(../libogg-1.3.1/include)

add_definitions(-DVERSION="1.2.0" -DFLAC_API_EXPORTS)

set(FLAC_SRC_FLAC
	src/flac/analyze.c
	src/flac/decode.c
	src/flac/encode.c
	src/flac/foreign_metadata.c
	src/flac/iffscan.c
	src/flac/local_string_utils.c
	src/flac/main.c
	src/flac/utils.c
	src/flac/vorbiscomment.c
)

set(FLAC_SRC_LIBFLAC
	src/libflac/bitmath.c
	src/libflac/bitreader.c
	src/libflac/bitwriter.c
	src/libflac/cpu.c
	src/libflac/crc.c
	src/libflac/fixed.c
	src/libflac/float.c
	src/libflac/format.c
	src/libflac/lpc.c
	src/libflac/md5.c
	src/libflac/memory.c
	src/libflac/metadata_iterators.c
	src/libflac/metadata_object.c
	src/libflac/ogg_decoder_aspect.c
	src/libflac/ogg_encoder_aspect.c
	src/libflac/ogg_helper.c
	src/libflac/ogg_mapping.c
	src/libflac/stream_decoder.c
	src/libflac/stream_encoder.c
	src/libflac/stream_encoder_framing.c
	src/libflac/window.c
)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	add_definitions(-DFLAC__CPU_PPC -DFLAC__USE_ALTIVEC -DFLAC__ALIGN_MALLOC_DATA)
	set(FLAC_SRC_LIBFLAC_NASM
		src/libflac/ppc/as/lpc_asm.s
	)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Solaris")
	add_definitions(-DFLAC__NO_ASM -DFLAC__ALIGN_MALLOC_DATA)
ELSE()
	add_definitions(-DFLAC__CPU_IA32 -DFLAC__USE_3DNOW -DFLAC__HAS_NASM -DFLAC__ALIGN_MALLOC_DATA)
	set(FLAC_SRC_LIBFLAC_NASM
		src/libflac/ia32/bitreader_asm.nasm
		src/libflac/ia32/cpu_asm.nasm
		src/libflac/ia32/fixed_asm.nasm
		src/libflac/ia32/lpc_asm.nasm
		src/libflac/ia32/stream_encoder_asm.nasm
	)
ENDIF()

nasm_command(FLAC_SRC_LIBFLAC_NASM FLAC_ASM_OUT)

set(FLAC_SOURCES ${FLAC_SRC_LIBFLAC})

include_directories("${FLAC_SOURCE_DIR}/include")
include_directories("${FLAC_SOURCE_DIR}/src/libflac/include")

add_library(FLAC SHARED
	${FLAC_SOURCES}
	${FLAC_ASM_OUT}
)

if(WIN32)
	set_property(TARGET FLAC APPEND_STRING PROPERTY
		COMPILE_FLAGS "/FI\"${FLAC_SOURCE_DIR}/src/win32_dll.h\""
	)
endif()

target_link_libraries (FLAC
	OGG
)

set_target_properties(FLAC
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY build)
set_target_properties(FLAC
	PROPERTIES PREFIX "" OUTPUT_NAME "libFLAC"
)
