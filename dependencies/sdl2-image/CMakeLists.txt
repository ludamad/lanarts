CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(SDL2_image)

ADD_DEFINITIONS (
    -Wno-attributes
    -DLOAD_PNG
)

INCLUDE_DIRECTORIES(../sdl2/include)
ADD_LIBRARY (
    SDL2-image

    IMG.c
    IMG_bmp.c
    IMG_gif.c
    IMG_jpg.c
    IMG_lbm.c
    IMG_pcx.c
    IMG_png.c
    IMG_pnm.c
    IMG_tga.c
    IMG_tif.c
    IMG_webp.c
    IMG_xcf.c
    IMG_xpm.c
    IMG_xv.c
    IMG_xxx.c
)

TARGET_LINK_LIBRARIES(
    SDL2-image
    png
)
