cmake_minimum_required(VERSION 2.9)

project(NCACommon)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")


set(NCACommon_SRCS
Array2d.cc
BlockMapping.cc
LUTMaker.cc
)

set(NCACommon_HEADERS
Array2d.h
BlockMapping.h
LUTmaker.h
)

# set everything needed for the root dictonary and create the
# dictionary
set(LINKDEF ${PROJECT_NAME}_LinkDef.h)
set(DICTIONARY ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Dict.cxx)
set(PCMFILE ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_rdict.pcm)
set(RMAPFILE ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.rootmap)
ROOT_GENERATE_DICTIONARY("${NCACommon_HEADERS}" "${LINKDEF}" "${DICTIONARY}" "${INCLUDE_DIRECTORIES}" "${PROJECT_NAME}")

# add the dictionary to the list of source files
SET(NCACommon_SRCS ${NCACommon_HEADERS} ${NCACommon_SRCS} ${DICTIONARY})

# Set the library version in the main CMakeLists.txt
SET(NCACommon_MAJOR_VERSION 0)
SET(NCACommon_MINOR_VERSION 0)
SET(NCACommon_PATCH_VERSION 0)
SET(NCACommon_VERSION "${NCACommon_MAJOR_VERSION}.${NCACommon_MINOR_VERSION}.${NCACommon_PATCH_VERSION}")
SET(NCACommon_LIBRARY_PROPERTIES ${NCACommon_LIBRARY_PROPERTIES}
    VERSION "${NCACommon_VERSION}"
    SOVERSION "${NCACommon_MAJOR_VERSION}"
    SUFFIX ".so"
)


############### build the library #####################
add_library(NCACommon SHARED ${NCACommon_SRCS})
target_link_libraries(NCACommon ${ROOT_LIBRARIES} )
set_target_properties(NCACommon PROPERTIES ${NCACommon_LIBRARY_PROPERTIES})

############### install the library ###################
install(TARGETS NCACommon DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${NCACommon_HEADERS} DESTINATION include/NCACommon)
install(FILES ${PCMFILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${RMAPFILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

