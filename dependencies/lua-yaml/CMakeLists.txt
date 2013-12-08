cmake_minimum_required( VERSION 2.6 )
PROJECT(project_luayaml)

set( luayaml_src
    api.c  b64.c  dumper.c  emitter.c  loader.c  lyaml.c  parser.c  reader.c  scanner.c  writer.c
)

include_directories("." "../lua")

add_library( luayaml STATIC
             ${luayaml_src} )
