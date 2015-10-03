cmake_minimum_required(VERSION 3.0)
project(viewer VERSION 0.0.0)

option(WARNINGS "Switch on extra warnings" OFF)
option(WERROR "Turn warnings into errors" OFF)

if(WARNINGS)
  set(WARNINGS_CXX_FLAGS ${WARNINGS_CXX_FLAGS}
    -pedantic -Wall -Wextra -Wno-c++0x-compat -Wnon-virtual-dtor
     -Wshadow -Wcast-qual -Winit-self -Wno-unused-parameter)
  # temporarily deactived: -Weffc++ -Wconversion
  # -Winline -Wfloat-equal -Wunreachable-code
endif()

if(WERROR)
  set(WARNINGS_CXX_FLAGS ${WARNINGS_CXX_FLAGS}
    -Werror)
endif()

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(Boost COMPONENTS system filesystem signals REQUIRED)
find_package(Threads REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_search_module(BLUEZ REQUIRED bluez)
pkg_search_module(GSTREAMERMM REQUIRED gstreamermm-1.0)
pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SDL2_IMAGE REQUIRED SDL2_image)
pkg_search_module(CAIROMM REQUIRED cairomm-1.0)

file(GLOB WIIC_SOURCES external/WiiC/src/wiic/*.c)
add_library(wiic ${WIIC_SOURCES})
target_compile_definitions(wiic PUBLIC -DLINUX=1)
target_include_directories(wiic SYSTEM PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/external/WiiC/src/log
  ${CMAKE_CURRENT_SOURCE_DIR}/external/WiiC/src/wiic)

file(GLOB WIICPP_SOURCES
  external/WiiC/src/wiicpp/*.cpp
  external/WiiC/src/log/*.cpp)
add_library(wiicpp ${WIICPP_SOURCES})
target_compile_definitions(wiicpp PUBLIC -DLINUX=1)
target_include_directories(wiicpp SYSTEM PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/external/WiiC/src/log
  ${CMAKE_CURRENT_SOURCE_DIR}/external/WiiC/src/wiic
  ${CMAKE_CURRENT_SOURCE_DIR}/external/WiiC/src/wiicpp)

file(GLOB VIEWER_SOURCES src/*.cpp)
add_executable(viewer ${VIEWER_SOURCES})
target_compile_options(viewer PRIVATE -std=c++1y ${WARNINGS_CXX_FLAGS})
target_include_directories(viewer SYSTEM PRIVATE
  ${BLUEZ_INCLUDE_DIRS}
  ${GSTREAMERMM_INCLUDE_DIRS}
  ${SDL2_INCLUDE_DIRS}
  ${SDL2_IMAGE_INCLUDE_DIRS}
  ${CAIROMM_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIR})
target_link_libraries(viewer
  wiicpp
  wiic
  bluetooth
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SIGNALS_LIBRARY}
  ${BLUEZ_LIBRARIES}
  ${GSTREAMERMM_LIBRARIES}
  ${SDL2_LIBRARIES}
  ${SDL2_IMAGE_LIBRARIES}
  ${CAIROMM_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${OPENGL_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT})

if(BUILD_TESTS)
  file(GLOB TEST_SOURCES test/*.cpp)
  foreach(SOURCE ${TEST_SOURCES})
    get_filename_component(SOURCE_BASENAME ${SOURCE} NAME_WE)
    add_executable(${SOURCE_BASENAME} ${SOURCE})
    target_link_libraries(${SOURCE_BASENAME} viewerlib)
  endforeach(SOURCE)
endif()

# EOF #
