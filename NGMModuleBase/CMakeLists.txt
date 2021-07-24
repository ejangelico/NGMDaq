cmake_minimum_required(VERSION 2.6)

project(NGMModuleBase)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

set( NGMModuleBase_SRCS
NGMModule.cc
NGMSystem.cc
)

set(NGMModuleBase_HEADERS
NGMModule.h
NGMSystem.h
)

# set everything needed for the root dictonary and create the
# dictionary
set(LINKDEF ${PROJECT_NAME}_LinkDef.h)
set(DICTIONARY ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Dict.cxx)
set(PCMFILE ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_rdict.pcm)
set(RMAPFILE ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.rootmap)
ROOT_GENERATE_DICTIONARY("${NGMModuleBase_HEADERS}" "${LINKDEF}" "${DICTIONARY}" "${INCLUDE_DIRECTORIES}" "${PROJECT_NAME}")

# add the dictionary to the list of source files
SET(NGMModuleBase_SRCS ${NGMModuleBase_HEADERS} ${NGMModuleBase_SRCS} ${DICTIONARY})

# Set the library version in the main CMakeLists.txt
SET(NGMModuleBase_MAJOR_VERSION 0)
SET(NGMModuleBase_MINOR_VERSION 0)
SET(NGMModuleBase_PATCH_VERSION 0)
SET(NGMModuleBase_VERSION "${NGMModuleBase_MAJOR_VERSION}.${NGMModuleBase_MINOR_VERSION}.${NGMModuleBase_PATCH_VERSION}")
SET(NGMModuleBase_LIBRARY_PROPERTIES ${NGMModuleBase_LIBRARY_PROPERTIES}
    VERSION "${NGMModuleBase_VERSION}"
    SOVERSION "${NGMModuleBase_MAJOR_VERSION}"
    SUFFIX ".so"
)

############### build the library #####################
add_library(NGMModuleBase SHARED ${NGMModuleBase_SRCS})
target_link_libraries(NGMModuleBase NGMData ${ROOT_LIBRARIES})
set_target_properties(NGMModuleBase PROPERTIES ${NGMModuleBase_LIBRARY_PROPERTIES})

############### install the library ###################
install(TARGETS NGMModuleBase DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${NGMModuleBase_HEADERS} DESTINATION include)
install(FILES ${PCMFILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${RMAPFILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

