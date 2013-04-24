cmake_minimum_required(VERSION 2.6)

add_definitions( -std=c++98 )

add_subdirectory( luawrap )
add_subdirectory( lanarts )
add_subdirectory( common )
add_subdirectory( draw )
add_subdirectory( net )
add_subdirectory( sound )


enable_testing()

