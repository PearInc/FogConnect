
cmake_minimum_required(VERSION 2.8.3)

project(FOG_CONNECT)

set(CMAKE_C_FLAGS "-O0 -ggdb ")

include_directories(${PROJECT_SOURCE_DIR})
include_directories(/usr/include/glib-2.0)


add_subdirectory(echo/)
add_subdirectory(filetransfer/)


add_library(fog_connection pr_fog_connect.c)
target_link_libraries(fog_connection 
                      pthread 
                      fogconnect 
                      fogquic 
                      usrsctp
                      glib-2.0
                      event
                      jansson
                      websockets 
                      crypto)