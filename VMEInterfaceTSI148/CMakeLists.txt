cmake_minimum_required(VERSION 2.9)

project(VMEInterfaceTSI148)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

include_directories(${CMAKE_INSTALL_PREFIX}/include)

include_directories(/usr/local/include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} " )
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  " )

link_directories( ${LINK_DIRECTORIES} ${CMAKE_INSTALL_PREFIX}/lib /usr/local/lib  /usr/local/linuxvmeen/cct_modules )

set( VMEInterfaceTSI148_SRCS
tsi148_vmeinterface.cc
)

set(VMEInterfaceTSI148_HEADERS
tsi148_vmeinterface.h
)

# set everything needed for the root dictonary and create the
# dictionary
set(LINKDEF ${PROJECT_NAME}_LinkDef.h)
set(DICTIONARY ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Dict.cxx)
set(PCMFILE ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_rdict.pcm)
set(RMAPFILE ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.rootmap)
ROOT_GENERATE_DICTIONARY("${VMEInterfaceTSI148_HEADERS}" "${LINKDEF}" "${DICTIONARY}" "${INCLUDE_DIRECTORIES}" "${PROJECT_NAME}")

# add the dictionary to the list of source files
SET(VMEInterfaceTSI148_SRCS ${VMEInterfaceTSI148_HEADERS} ${VMEInterfaceTSI148_SRCS} ${DICTIONARY})

# Set the library version in the main CMakeLists.txt
SET(VMEInterfaceTSI148_MAJOR_VERSION 0)
SET(VMEInterfaceTSI148_MINOR_VERSION 0)
SET(VMEInterfaceTSI148_PATCH_VERSION 0)
SET(VMEInterfaceTSI148_VERSION "${VMEInterfaceTSI148_MAJOR_VERSION}.${VMEInterfaceTSI148_MINOR_VERSION}.${VMEInterfaceTSI148_PATCH_VERSION}")
SET(VMEInterfaceTSI148_LIBRARY_PROPERTIES ${VMEInterfaceTSI148_LIBRARY_PROPERTIES}
    VERSION "${VMEInterfaceTSI148_VERSION}"
    SOVERSION "${VMEInterfaceTSI148_MAJOR_VERSION}"
    SUFFIX ".so"
)

############### build the library #####################
add_library(VMEInterfaceTSI148 SHARED ${VMEInterfaceTSI148_SRCS})
target_link_libraries(VMEInterfaceTSI148 VMECards cctvmeen ${ROOT_LIBRARIES})
set_target_properties(VMEInterfaceTSI148 PROPERTIES ${VMEInterfaceTSI148_LIBRARY_PROPERTIES})

add_custom_command(TARGET VMEInterfaceTSI148 POST_BUILD COMMAND rlibmap -o ${RMAPFILE} -l libVMEInterfaceTSI148 -c ${CMAKE_CURRENT_SOURCE_DIR}/${VMEInterfaceTSI148_LINKDEF} COMMENT Install libNGMDaq.rootmap ) 

############### install the library ###################
install(TARGETS VMEInterfaceTSI148 DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${RMAPFILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${PCMFILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${RMAPFILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

