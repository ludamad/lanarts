# Copyright 2014 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
cmake_minimum_required(VERSION 2.8.7)

project(libvorbis)

if(WIN32)
  add_definitions(-D_USE_MATH_DEFINES)
endif()

# Include libogg header files.
include_directories(../libogg-1.3.1/lib/include)

# Include libvorbis header files.
include_directories(include)
include_directories(lib)

set(libvorbis_SRCS
    ./lib/info.c
    ./lib/codebook.c
    ./lib/registry.c
    ./lib/mdct.c
    ./lib/res0.c
    ./lib/mapping0.c
    ./lib/envelope.c
    ./lib/window.c
    ./lib/synthesis.c
    ./lib/psy.c
    ./lib/lpc.c
    ./lib/vorbisenc.c
    ./lib/lookup.c
    ./lib/analysis.c
    ./lib/floor1.c
    ./lib/floor0.c
    ./lib/lsp.c
    ./lib/barkmel.c
    ./lib/tone.c
    ./lib/block.c
    ./lib/smallft.c
    ./lib/vorbisfile.c
    ./lib/bitrate.c
    ./lib/sharedbook.c
    ../libogg-1.3.1/src/bitwise.c
    ../libogg-1.3.1/src/framing.c

)
    # ${libvorbis_dir}/lib/tone.c

add_library(vorbis STATIC ${libvorbis_SRCS})

