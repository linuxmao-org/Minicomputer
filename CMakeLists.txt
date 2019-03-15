cmake_minimum_required(VERSION "3.3")

include(FindPkgConfig)

set(FLTK_SKIP_OPENGL ON)
set(FLTK_SKIP_FORMS ON)
set(FLTK_SKIP_IMAGES ON)
set(FLTK_SKIP_FLUID ON)
find_package(FLTK REQUIRED)

if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
  string(APPEND CMAKE_C_FLAGS " -Wall")
  string(APPEND CMAKE_C_FLAGS " -ffast-math")
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang)$")
  string(APPEND CMAKE_CXX_FLAGS " -Wall")
  string(APPEND CMAKE_CXX_FLAGS " -ffast-math")
endif()

function(target_link_pkgconfig TARGET VAR NAME VISIBILITY)
  pkg_check_modules("${VAR}" "${NAME}" REQUIRED)
  target_include_directories("${TARGET}" "${VISIBILITY}" ${${VAR}_INCLUDE_DIRS})
  target_link_libraries("${TARGET}" "${VISIBILITY}" ${${VAR}_LIBRARIES})
endfunction()

add_executable(minicomputer
  "editor/main.cpp"
  "editor/Memory.cpp"
  "editor/syntheditor.cxx")
target_link_pkgconfig(minicomputer LO "liblo" PRIVATE)
target_link_pkgconfig(minicomputer ASOUND "alsa" PRIVATE)
target_include_directories(minicomputer PRIVATE "${FLTK_INCLUDE_DIR}")
target_link_libraries(minicomputer PRIVATE ${FLTK_LIBRARIES})

add_executable(minicomputerCPU
  "cpu/main.c")
target_link_pkgconfig(minicomputerCPU JACK "jack" PRIVATE)
target_link_pkgconfig(minicomputerCPU LO "liblo" PRIVATE)
target_link_pkgconfig(minicomputerCPU ASOUND "alsa" PRIVATE)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  find_library(MATH_LIBRARY "m")
  target_link_libraries(minicomputerCPU PRIVATE "${MATH_LIBRARY}")
endif()
