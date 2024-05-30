cmake_minimum_required(VERSION 2.9)

project(SIS3316SystemMT)

#set(CMAKE_SKIP_RPATH true)

#Check the compiler and set the compile and link flags
#set(CMAKE_BUILD_TYPE Debug)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR} "$ENV{TBBLIBDIR}"
)

link_directories( ${LINK_DIRECTORIES})
if(EXISTS ENV{TBBINCDIR})
  include_directories( "$ENV{TBBINCDIR}" ${CMAKE_CURRENT_SOURCE_DIR}  )
else()
  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}  )
endif(EXISTS ENV{TBBINCDIR})

set( SIS3316SystemMT_SRCS
SIS3316SystemMT.cc
)

set(SIS3316SystemMT_HEADERS
SIS3316SystemMT.h
)

# set everything needed for the root dictionary and create the
# dictionary
set(LINKDEF ${PROJECT_NAME}_LinkDef.h)
set(DICTIONARY ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Dict.cxx)
set(PCMFILE ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_rdict.pcm)
set(RMAPFILE ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.rootmap)
ROOT_GENERATE_DICTIONARY("${SIS3316SystemMT_HEADERS}" "${LINKDEF}" "${DICTIONARY}" "${INCLUDE_DIRECTORIES};${PROJECT_SOURCE_DIR}" "${PROJECT_NAME}")

# add the dictionary to the list of source files
SET(SIS3316SystemMT_SRCS ${SIS3316SystemMT_HEADERS} ${SIS3316SystemMT_SRCS} ${DICTIONARY})

# Set the library version in the main CMakeLists.txt
SET(SIS3316SystemMT_MAJOR_VERSION 0)
SET(SIS3316SystemMT_MINOR_VERSION 0)
SET(SIS3316SystemMT_PATCH_VERSION 0)
SET(SIS3316SystemMT_VERSION "${SIS3316SystemMT_MAJOR_VERSION}.${SIS3316SystemMT_MINOR_VERSION}.${SIS3316SystemMT_PATCH_VERSION}")
SET(SIS3316SystemMT_LIBRARY_PROPERTIES ${SIS3316SystemMT_LIBRARY_PROPERTIES}
    VERSION "${SIS3316SystemMT_VERSION}"
    SOVERSION "${SIS3316SystemMT_MAJOR_VERSION}"
    SUFFIX ".so"
)

############### build the library #####################
add_library(SIS3316SystemMT SHARED ${SIS3316SystemMT_SRCS})
target_link_libraries(SIS3316SystemMT NGMModuleBase VMECards tbb ${ROOT_LIBRARIES})
set_target_properties(SIS3316SystemMT PROPERTIES ${SIS3316SystemMT_LIBRARY_PROPERTIES})

############### install the library ###################
install(TARGETS SIS3316SystemMT DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${PCMFILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${RMAPFILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

