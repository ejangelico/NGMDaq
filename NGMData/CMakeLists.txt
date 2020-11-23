cmake_minimum_required(VERSION 2.6)

project(NGMData)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

set( NGMData_SRCS
NGMBufferedPacket.cc
NGMConfigurationParameter.cc
NGMConfigurationTable.cc
NGMHit.cc
NGMLogger.cc
NGMSystemConfiguration.cc
NGMWaveform.cc
)

set(NGMData_HEADERS
NGMBufferedPacket.h
NGMConfigurationParameter.h
NGMConfigurationTable.h
NGMHit.h
NGMLogger.h
NGMSystemConfiguration.h
NGMWaveform.h
)

# set everything needed for the root dictonary and create the
# dictionary
set(LINKDEF ${PROJECT_NAME}_LinkDef.h)
set(DICTIONARY ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Dict.cxx)
set(PCMFILE ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_rdict.pcm)
set(RMAPFILE ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.rootmap)
ROOT_GENERATE_DICTIONARY("${NGMData_HEADERS}" "${LINKDEF}" "${DICTIONARY}" "${INCLUDE_DIRECTORIES}" "${PROJECT_NAME}")

# add the dictionary to the list of source files
SET(NGMData_SRCS ${NGMData_HEADERS} ${NGMData_SRCS} ${DICTIONARY})

# Set the library version in the main CMakeLists.txt
SET(NGMData_MAJOR_VERSION 0)
SET(NGMData_MINOR_VERSION 0)
SET(NGMData_PATCH_VERSION 0)
SET(NGMData_VERSION "${NGMData_MAJOR_VERSION}.${NGMData_MINOR_VERSION}.${NGMData_PATCH_VERSION}")
SET(NGMData_LIBRARY_PROPERTIES ${NGMData_LIBRARY_PROPERTIES}
    VERSION "${NGMData_VERSION}"
    SOVERSION "${NGMData_MAJOR_VERSION}"
    SUFFIX ".so"
)


############### build the library #####################
add_library(NGMData SHARED ${NGMData_SRCS})
target_link_libraries(NGMData ${ROOT_LIBRARIES} )
set_target_properties(NGMData PROPERTIES ${NGMData_LIBRARY_PROPERTIES})

set(INCLUDE_FLAGS)
foreach (_current_FILE ${INCLUDE_DIRECTORIES})
  set(INCLUDE_FLAGS ${INCLUDE_FLAGS} -I${_current_FILE})
endforeach (_current_FILE ${INCLUDE_DIRECTORIES})

############### install the library ###################
install(TARGETS NGMData DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${NGMData_HEADERS} DESTINATION include)
install(FILES ${PCMFILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${RMAPFILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

