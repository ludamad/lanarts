cmake_minimum_required( VERSION 2.6 )
PROJECT(project_luasocket)

set( luasocket_src
    auxiliar.c buffer.c except.c inet.c io.c luasocket.c mime.c options.c select.c tcp.c timeout.c udp.c unix.c usocket.c wsocket.c
)

add_library( luasocket STATIC
             ${luasocket_src} )
