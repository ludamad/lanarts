cmake_minimum_required( VERSION 2.6 )
PROJECT(project_polypartition)

set( polypartition_src
    polypartition.cpp
)

add_library( polypartition STATIC
             ${polypartition_src} )
