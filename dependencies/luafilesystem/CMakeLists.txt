cmake_minimum_required(VERSION 3.0)
project(luafilesystem)

set(LFS_INCLUDES ../lua)

add_library( lfs STATIC src/lfs.h src/lfs.c src/lfs.def)

target_include_directories(lfs PRIVATE ${LFS_INCLUDES})

install(TARGETS lfs
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
