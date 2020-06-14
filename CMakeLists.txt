cmake_minimum_required(VERSION 2.8)

add_compile_options( -Wall -std=c++17 )

project(curlstats)

set ( release_tag "0.0.1" )

# show build environment
site_name( SITE_NAME )
message( STATUS "SITE_NAME              : ${SITE_NAME}")
message( STATUS "CMAKE_SYSTEM           : ${CMAKE_SYSTEM}")
message( STATUS "CMAKE_SYSTEM_NAME      : ${CMAKE_SYSTEM_NAME}")
message( STATUS "CMAKE_SYSTEM_PROCESSOR : ${CMAKE_SYSTEM_PROCESSOR}")
message( STATUS "CMAKE_CXX_COMPILER     : ${CMAKE_CXX_COMPILER}")
message( STATUS "CMAKE_HOST_SYSTEM      : ${CMAKE_HOST_SYSTEM}")
message( STATUS "CMAKE_VERSION          : ${CMAKE_VERSION}")


set(  curlstats_objects
      src/main.cpp
)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR} )


add_executable( curlstats ${curlstats_objects} )
