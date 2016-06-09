cmake_minimum_required(VERSION 3.0)

add_compile_options(-std=c++11 -Wall)

project(pommade CXX)

# iwyu: "include-what-you-use" suggests include/forward declaration optimizations during compilation
if (BUILD_IWYU)
  include(cmake/Modules/IWYU.cmake)
  iwyu_enable()
  find_program(IWYU_PATH NAMES include-what-you-use iwyu)
  if (NOT IWYU_PATH)
    message(FATAL_ERROR "include-what-you-use (iwyu) not found")
  endif ()
endif ()

file(GLOB CC_FILES *.cc)

# prefer static to dynamic libraries
set(CMAKE_FIND_LIBRARY_SUFFIXES .a)

find_package(PkgConfig)
find_package(XercesC REQUIRED)
find_package(Threads REQUIRED)

set(Boost_USE_STATIC_LIBS ON)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  find_library(ICUUC_LIBS sicuuc)
  find_library(ICUDATA_LIBS sicudt)
else ()
  find_library(ODBC_LIBS odbc)
  find_library(ICUUC_LIBS icuuc)
  find_library(ICUDATA_LIBS icudata)
endif ()

add_executable(pommade ${CC_FILES})

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  target_link_libraries(pommade ${XercesC_LIBRARY} ${ICUUC_LIBS} ${ICUDATA_LIBS} libstdc++.a libgcc_eh.a libodbc32.dll ${CMAKE_THREAD_LIBS_INIT})
else ()
  target_link_libraries(pommade ${XercesC_LIBRARY} ${ICUUC_LIBS} ${ICUDATA_LIBS} ${CMAKE_THREAD_LIBS_INIT} -lltdl -ldl)  
endif ()
