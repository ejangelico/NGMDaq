cmake_minimum_required(VERSION 2.6)

project(SIS3316System)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

set( SIS3316System_SRCS
SIS3316System.cc
)

set(SIS3316System_HEADERS
SIS3316System.h
)

# set everything needed for the root dictionary and create the
# dictionary
set(LINKDEF ${PROJECT_NAME}_LinkDef.h)
set(DICTIONARY ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Dict.cxx)
set(PCMFILE ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_rdict.pcm)
set(RMAPFILE ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.rootmap)
ROOT_GENERATE_DICTIONARY("${SIS3316System_HEADERS}" "${LINKDEF}" "${DICTIONARY}" "${INCLUDE_DIRECTORIES};${PROJECT_SOURCE_DIR}" "${PROJECT_NAME}")

# add the dictionary to the list of source files
SET(SIS3316System_SRCS ${SIS3316System_HEADERS} ${SIS3316System_SRCS} ${DICTIONARY})

# Set the library version in the main CMakeLists.txt
SET(SIS3316System_MAJOR_VERSION 0)
SET(SIS3316System_MINOR_VERSION 0)
SET(SIS3316System_PATCH_VERSION 0)
SET(SIS3316System_VERSION "${SIS3316System_MAJOR_VERSION}.${SIS3316System_MINOR_VERSION}.${SIS3316System_PATCH_VERSION}")
SET(SIS3316System_LIBRARY_PROPERTIES ${SIS3316System_LIBRARY_PROPERTIES}
    VERSION "${SIS3316System_VERSION}"
    SOVERSION "${SIS3316System_MAJOR_VERSION}"
    SUFFIX ".so"
)

############### build the library #####################
add_library(SIS3316System SHARED ${SIS3316System_SRCS})
target_link_libraries(SIS3316System NGMModuleBase VMECards ${ROOT_LIBRARIES})
set_target_properties(SIS3316System PROPERTIES ${SIS3316System_LIBRARY_PROPERTIES})

############### install the library ###################
install(TARGETS SIS3316System DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${PCMFILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${RMAPFILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

