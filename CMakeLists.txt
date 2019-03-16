cmake_minimum_required(VERSION "3.3")

set(GETTEXT_TRANSLATIONS "fr")

include(GNUInstallDirs)
include(FindPkgConfig)

set(FLTK_SKIP_OPENGL ON)
set(FLTK_SKIP_FORMS ON)
set(FLTK_SKIP_IMAGES ON)
set(FLTK_SKIP_FLUID ON)
find_package(FLTK REQUIRED)

set(CMAKE_CXX_STANDARD "11")

if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffast-math")
  #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang)$")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math")
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
endif()

function(target_link_pkgconfig TARGET VAR NAME VISIBILITY)
  pkg_check_modules("${VAR}" "${NAME}" REQUIRED)
  target_include_directories("${TARGET}" "${VISIBILITY}" ${${VAR}_INCLUDE_DIRS})
  target_link_libraries("${TARGET}" "${VISIBILITY}" ${${VAR}_LIBRARIES})
endfunction()

add_executable(minicomputer
  "editor/main.cpp"
  "editor/Memory.cpp"
  "editor/syntheditor.cxx"
  "cpu/cpu.cpp"
  "cpu/main.cpp")
target_link_pkgconfig(minicomputer JACK "jack" PRIVATE)
target_include_directories(minicomputer PRIVATE "${FLTK_INCLUDE_DIR}")
target_link_libraries(minicomputer PRIVATE ${FLTK_LIBRARIES})
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  find_library(MATH_LIBRARY "m")
  target_link_libraries(minicomputer PRIVATE "${MATH_LIBRARY}")
  find_library(DL_LIBRARY "dl")
  target_link_libraries(minicomputer PRIVATE "${DL_LIBRARY}")
endif()

include(FindThreads)
target_link_libraries(minicomputer PRIVATE "${CMAKE_THREAD_LIBS_INIT}")

target_compile_definitions(minicomputer PRIVATE "MINICOMPUTER_LOCALEDIR=\"${CMAKE_INSTALL_FULL_LOCALEDIR}\"")
install(TARGETS minicomputer DESTINATION "${CMAKE_INSTALL_BINDIR}")

set(ENABLE_GETTEXT "" CACHE STRING "Enable gettext")
if(ENABLE_GETTEXT STREQUAL "")
  find_package(Intl)
  find_package(Gettext)
  if(Intl_FOUND AND GETTEXT_FOUND)
    set(ENABLE_GETTEXT "ON" CACHE STRING "Enable gettext" FORCE)
  endif()
endif()
if(ENABLE_GETTEXT)
  find_package(Intl REQUIRED)
  find_package(Gettext REQUIRED)
endif()

if(ENABLE_GETTEXT)
  target_compile_definitions(minicomputer PRIVATE "MINICOMPUTER_I18N=1")
  target_include_directories(minicomputer PRIVATE ${Intl_INCLUDE_DIRS})
  target_link_libraries(minicomputer PRIVATE ${Intl_LIBRARIES})
  foreach(translation ${GETTEXT_TRANSLATIONS})
    gettext_process_po_files("${translation}" ALL INSTALL_DESTINATION "${CMAKE_INSTALL_LOCALEDIR}" PO_FILES
      "po/${translation}/minicomputer.po")
  endforeach()
endif()

install(FILES "minicomputer.xpm" DESTINATION "${CMAKE_INSTALL_DATADIR}/pixmaps")
install(FILES "minicomputer.desktop" DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")

message("!! Feature summary:")
macro(print_feature NAME VAR)
  if(${VAR})
    message("!!   ${NAME}: YES")
  else()
    message("!!   ${NAME}: NO")
  endif()
endmacro()
print_feature("gettext" ENABLE_GETTEXT)
