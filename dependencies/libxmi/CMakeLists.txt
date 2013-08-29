cmake_minimum_required( VERSION 2.6 )
PROJECT(project_libxmi)

include_directories(".")

set( libxmi_src
    config.h
    drawing.c
    extern.h
    mi_alloc.c
    mi_api.c
    mi_api.h
    mi_arc.c
    mi_arc.h
    mi_canvas.c
    mi_fllarc.c
    mi_fllarc.h
    mi_fllrct.c
    mi_fplycon.c
    mi_fply.h
    mi_gc.c
    mi_gc.h
    mi_line.h
    mi_ply.c
    mi_plycon.c
    mi_plygen.c
    mi_ply.h
    mi_plypnt.c
    mi_plyutil.c
    mi_scanfill.h
    mi_spans.c
    mi_spans.h
    mi_version.c
    mi_widelin.c
    mi_widelin.h
    mi_zerarc.c
    mi_zerarc.h
    mi_zerolin.c
    sys-defines.h
    xmi.h
)

add_library( libxmi STATIC
             ${libxmi_src} )
