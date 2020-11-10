cmake_minimum_required(VERSION 2.8)

add_compile_options( -Wall -std=c++17 )

set( PROJECT "curlstats")

project(${PROJECT})

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
      src/comments.cpp
      src/curlprobe.cpp
      src/datetime.cpp
      src/html.cpp
      src/main.cpp
      src/options.cpp
      src/output.cpp
      src/qtystats.cpp
      src/text.cpp
      src/waitclass.cpp
      src/util.cpp
      src/variables.cpp
)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR} )


add_executable( curlstats ${curlstats_objects} )

find_package(Doxygen)
if (DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/src/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message( STATUS "Doxygen configured")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target( doc
        COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/doxygen && ${DOXYGEN_EXECUTABLE} -d markdown ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )

else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)
