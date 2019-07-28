cmake_minimum_required(VERSION 3.1.0)
project(CG_Mining_Bot)

set(CMAKE_CXX_STANDARD 11)
SET(requiredlibs)

set(CURL_LIBRARY "-lcurl")
find_package(CURL REQUIRED)

include_directories(${CURL_INCLUDE_DIR})

cmake_minimum_required (VERSION 2.6)
find_package (Threads)

add_executable(CG_Mining_Bot src/main.cpp src/helper.cpp src/minerapi.cpp src/data.cpp src/ngui.cpp)

target_link_libraries(CG_Mining_Bot ${CURL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ncurses menu)


