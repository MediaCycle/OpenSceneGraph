#---
# File: FindPDFium.cmake
#
# Find the native PDFium includes and library
#
#  PDFium_INCLUDE_DIR - where to find liblas's includes.
#  PDFium_LIBRARY    - List of libraries when using liblas.
#  PDFium_FOUND        - True if liblas found.
#---
#
# Copyright 2021 Christian Frisson
# Copyright 2017 Benoît Blanchon
#
#---
# Adapted from PDFiumConfig.cmake by Benoît Blanchon
#
# https://github.com/bblanchon/pdfium-binaries/blob/master/PDFiumConfig.cmake 
#
# PDFium Package Configuration for CMake
#
# To use PDFium in you CMake project:
#
#   1. set the environment variable PDFium_DIR to the folder containing this file.
#   2. in your CMakeLists.txt, add
#       find_package(PDFium)
#   3. then link you excecutable with PDFium
#       target_link_libraries(my_exe pdfium)

message("ENV{PDFium_DIR} $ENV{PDFium_DIR}")

include(FindPackageHandleStandardArgs)

find_path(PDFium_INCLUDE_DIR
    NAMES "fpdfview.h"
    PATHS "${CMAKE_CURRENT_LIST_DIR}" "$ENV{PDFium_DIR}"
    PATH_SUFFIXES "include"
)

if(MSVC)
  if(CMAKE_CL_64)
    set(PDFium_ARCH x64)
  else()
    set(PDFium_ARCH x86)
  endif()

  find_file(PDFium_LIBRARY
        NAMES "pdfium.dll"
        PATHS "${CMAKE_CURRENT_LIST_DIR}" "$ENV{PDFium_DIR}"
        PATH_SUFFIXES "${PDFium_ARCH}/bin")

  find_file(PDFium_IMPLIB
        NAMES "pdfium.dll.lib"
        PATHS "${CMAKE_CURRENT_LIST_DIR}" "$ENV{PDFium_DIR}"
        PATH_SUFFIXES "${PDFium_ARCH}/lib")

  add_library(pdfium SHARED IMPORTED)
  set_target_properties(pdfium
    PROPERTIES
    IMPORTED_LOCATION             "${PDFium_LIBRARY}"
    IMPORTED_IMPLIB               "${PDFium_IMPLIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${PDFium_INCLUDE_DIR};${PDFium_INCLUDE_DIR}/cpp"
  )

  find_package_handle_standard_args(PDFium
    REQUIRED_VARS PDFium_LIBRARY PDFium_IMPLIB PDFium_INCLUDE_DIR
  )
else()
  find_library(PDFium_LIBRARY
        NAMES "pdfium"
        PATHS "${CMAKE_CURRENT_LIST_DIR}" "$ENV{PDFium_DIR}"
        PATH_SUFFIXES "lib")

  add_library(pdfium SHARED IMPORTED)
  set_target_properties(pdfium
    PROPERTIES
    IMPORTED_LOCATION             "${PDFium_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${PDFium_INCLUDE_DIR};${PDFium_INCLUDE_DIR}/cpp"
  )

  find_package_handle_standard_args(PDFium
    REQUIRED_VARS PDFium_LIBRARY PDFium_INCLUDE_DIR
  )
endif()

set(PDFium_FOUND "NO")
if(PDFium_LIBRARY AND PDFium_INCLUDE_DIR)
    set(PDFium_LIBRARIES ${PDFium_LIBRARY} )
    set(PDFium_FOUND "YES")
endif()
