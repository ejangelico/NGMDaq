cmake_minimum_required(VERSION 2.9)

project(NGMDaq)

set(CMAKE_VERBOSE_MAKEFILE on)
include(FindROOT.cmake)

include(CheckIncludeFile)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++20" COMPILER_SUPPORTS_CXX17)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX17)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
elseif(COMPILER_SUPPORTS_CXX0X)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no c++17 support. Please use a different C++ compiler.")
endif()

set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR}
${CMAKE_SOURCE_DIR}/NCACommon 
${CMAKE_SOURCE_DIR}/NGMData 
${CMAKE_SOURCE_DIR}/NGMModuleBase 
${CMAKE_SOURCE_DIR}/NGMModuleCore
${CMAKE_SOURCE_DIR}/NGMAnalysis
${CMAKE_SOURCE_DIR}/NGMBlockDetector
${CMAKE_SOURCE_DIR}/VMECards
${CMAKE_SOURCE_DIR}/VMEInterfaces
${CMAKE_SOURCE_DIR}/Systems/SIS3316System
${CMAKE_SOURCE_DIR}/Systems/SIS3302System
${CMAKE_SOURCE_DIR}/VMEInterfaceTSI148
${TBBROOT}/include
${TBBINCDIR}
)

include_directories( ${INCLUDE_DIRECTORIES})

add_subdirectory(NGMData)
add_subdirectory(NCACommon)
add_subdirectory(NGMModuleBase)
add_subdirectory(NGMModuleCore)
add_subdirectory(NGMAnalysis)
add_subdirectory(NGMBlockDetector)
add_subdirectory(VMECards)

find_library(USB_LIBRARY NAMES libusb usb
             HINTS ${NGM} ${CMAKE_INSTALL_PREFIX} )
    
set(HAVE_3150 1)
if(${USB_LIBRARY} MATCHES USB_LIBRARY-NOTFOUND)
    MESSAGE( "USB_LIBRARY not found" )
    set(HAVE_3150 0)
endif(${USB_LIBRARY} MATCHES USB_LIBRARY-NOTFOUND)

if(HAVE_3150)
add_subdirectory(VMEInterfaces)
endif(HAVE_3150)

add_subdirectory(Systems/SIS3316System)
add_subdirectory(Systems/SIS3302System)

#if(HAVETBB)
add_subdirectory(Systems/SIS3316SystemMT)
#endif(HAVETBB)

if(HAVETSI)
add_subdirectory(VMEInterfaceTSI148)
endif(HAVETSI)


