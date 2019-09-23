# grumgl - Grumbel's Random Collection of OpenGL Experiments
# Copyright (C) 2012-2019 Ingo Ruhnke <grumbel@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.0)
project(grumgl VERSION 0.0.0)

include(GNUInstallDirs)

option(WARNINGS "Switch on extra warnings" OFF)
option(WERROR "Turn warnings into errors" OFF)
option(USE_OPENGLES2 "Use OpenGLES2 instead of OpenGL" OFF)

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

if(EXISTS /opt/vc/include/bcm_host.h)
  set(HAVE_BCMHOST TRUE)
  message(STATUS "Looking for /opt/vc/include/bcm_host.h - found")
else()
  message(STATUS "Looking for /opt/vc/include/bcm_host.h - not found")
endif()

if(USE_OPENGLES2 OR HAVE_BCMHOST)
  message(STATUS "Using OpenGLES2")
  set(OPENGL_LIBPATH /opt/vc/lib)
  set(OPENGL_INCLUDE_DIR /opt/vc/include)
  set(OPENGL_LIBRARIES GLESv2)
  set(OPENGL_CFLAGS_OTHER -DHAVE_OPENGLES2)

  find_library(OPENGL_LIBRARIES GLESv2 ${OPENGL_LIBPATH})
else()
  message(STATUS "Using OpenGL3.3")
  set(OpenGL_GL_PREFERENCE "LEGACY")
  find_package(OpenGL REQUIRED)
  find_package(GLEW REQUIRED)
endif()

find_package(Boost REQUIRED COMPONENTS system filesystem)
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

file(GLOB GRUMGL_SOURCES src/*.cpp)
add_executable(grumgl ${GRUMGL_SOURCES})
target_compile_options(grumgl PRIVATE -std=c++1y ${WARNINGS_CXX_FLAGS})
target_compile_definitions(grumgl PRIVATE ${OPENGL_CFLAGS_OTHER})
target_include_directories(grumgl SYSTEM PRIVATE
  ${BLUEZ_INCLUDE_DIRS}
  ${GSTREAMERMM_INCLUDE_DIRS}
  ${SDL2_INCLUDE_DIRS}
  ${SDL2_IMAGE_INCLUDE_DIRS}
  ${CAIROMM_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIR})
target_link_libraries(grumgl
  wiicpp
  wiic
  bluetooth
  Boost::filesystem
  ${BLUEZ_LIBRARIES}
  ${GSTREAMERMM_LIBRARIES}
  ${SDL2_LIBRARIES}
  ${SDL2_IMAGE_LIBRARIES}
  ${CAIROMM_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS grumgl
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  RENAME grumgl)

install(PROGRAMS tools/blenderexp.py
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  RENAME grumgl-blender-export)

install(PROGRAMS tools/splitcubemap.py
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  RENAME grumgl-splitcubemap)

file(GLOB SHADER_FILES src/glsl/*.frag src/glsl/*.vert)
install(FILES ${SHADER_FILES}
  DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/glsl)

if(BUILD_TESTS)
  file(GLOB TEST_SOURCES test/*.cpp)
  foreach(SOURCE ${TEST_SOURCES})
    get_filename_component(SOURCE_BASENAME ${SOURCE} NAME_WE)
    add_executable(${SOURCE_BASENAME} ${SOURCE})
    target_link_libraries(${SOURCE_BASENAME} grumgllib)
  endforeach(SOURCE)
endif()

if(BUILD_BENCHMARKS)
  # build google-benchmark
  include_directories(${PROJECT_SOURCE_DIR}/src)
  set(BENCHMARK_SOURCES
    "external/benchmark/src/benchmark.cc"
    "external/benchmark/src/colorprint.cc"
    "external/benchmark/src/commandlineflags.cc"
    "external/benchmark/src/console_reporter.cc"
    "external/benchmark/src/csv_reporter.cc"
    "external/benchmark/src/json_reporter.cc"
    "external/benchmark/src/log.cc"
    "external/benchmark/src/reporter.cc"
    "external/benchmark/src/sleep.cc"
    "external/benchmark/src/string_util.cc"
    "external/benchmark/src/sysinfo.cc"
    "external/benchmark/src/walltime.cc"
    "external/benchmark/src/re_std.cc")
  add_library(benchmark ${BENCHMARK_SOURCES})
  target_compile_definitions(benchmark PUBLIC -DNDEBUG -DHAVE_STD_REGEX)
  target_compile_options(benchmark PRIVATE -std=c++1y)
  target_include_directories(benchmark SYSTEM PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/benchmark/include)

  # build benchmarks
  file(GLOB BENCHMARKSOURCES benchmarks/*.cpp)
  foreach(SOURCE ${BENCHMARKSOURCES})
    get_filename_component(SOURCE_BASENAME ${SOURCE} NAME_WE)
    add_executable(${SOURCE_BASENAME} ${SOURCE})
    target_link_libraries(${SOURCE_BASENAME} benchmark ${CMAKE_THREAD_LIBS_INIT})
    set_target_properties(${SOURCE_BASENAME} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/benchmarks/")
    target_compile_options(${SOURCE_BASENAME} PRIVATE -std=c++1y ${WARNINGS_CXX_FLAGS})
  endforeach(SOURCE)
endif()

# EOF #
