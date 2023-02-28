cmake_minimum_required( VERSION 2.8 )

project( Webserver 
         DESCRIPTION "The client-server application"
         LANGUAGES CXX)

set( "OpenCV_DIR" "${CMAKE_CURRENT_SOURCE_DIR}/build" )
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

set( CMAKE_THREAD_PREFER_PTHREAD TRUE )
set( THREADS_PREFER_THREAD_FLAG TRUE )
find_package( Threads REQUIRED )

add_executable( Client Client.cpp )
add_executable( Server Server.cpp )

target_link_libraries( Client ${OpenCV_LIBS} )
target_link_libraries( Server ${OpenCV_LIBS} )
target_link_libraries( Server Threads::Threads )

