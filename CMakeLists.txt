cmake_minimum_required(VERSION 3.5)

project(millon-server C CXX ASM)

set(SYS_LIBS -lm -lpthread -ldl)
ADD_DEFINITIONS(
    -DNGX_LINUX
    -DUNIX -g -fPIC -Wunused-variable -Wunused-parameter -Wunused-function -Wunused
    -Wno-deprecated -Wwrite-strings
)

set(BIN_DIR ${PROJECT_SOURCE_DIR}/bin)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR} )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BIN_DIR} )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BIN_DIR} )

foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BIN_DIR} )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BIN_DIR} )
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BIN_DIR} )
    message(STATUS CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG})
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

link_directories(${BIN_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src/)
include_directories(${PROJECT_SOURCE_DIR}/src/base)
include_directories(${PROJECT_SOURCE_DIR}/src/mqtt)

include_directories(${PROJECT_SOURCE_DIR}/3rd/libco/src)
include_directories(${PROJECT_SOURCE_DIR}/3rd/nginx/src/src/os)
include_directories(${PROJECT_SOURCE_DIR}/3rd/nginx/src/src/core)
include_directories(${PROJECT_SOURCE_DIR}/3rd/nginx/src/src/event)
include_directories(${PROJECT_SOURCE_DIR}/3rd/nginx/src/objs)
include_directories(${PROJECT_SOURCE_DIR}/3rd/nginx/src/src/os/unix)
include_directories(${PROJECT_SOURCE_DIR}/3rd/mqtt/src/MQTTPacket/src)

add_subdirectory(3rd/libco)
add_subdirectory(3rd/nginx)
add_subdirectory(3rd/mqtt)
add_subdirectory(src/base)
add_subdirectory(src/echo)
add_subdirectory(src/mqtt)


