cmake_minimum_required(VERSION 2.6)

project(VMECards)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

set( VMECards_SRCS
vme_interface_class.cc
sis3316_ethernetB_access_class.cc
sis3316_ethernet_access_class.cc
sis3316card.cc
sis3302card.cc
sis3820card.cc
caenHV6533card.cc
)

set(VMECards_HEADERS
vme_interface_class.h
sis3316_ethernetB_access_class.h
sis3316_ethernet_access_class.h
sis3316card.h
sis3316.h
sis3302card.h
sis3302.h
sis3820card.h
caenHV6533card.h
)

set( SIS3316SimpleTest_SRCS
SIS3316SimpleTest.cc
)


set( SIS3350Scope_SRCS
sis3350_scope.cc
)

set (SIS3316EthTest_SRCS
sis3316_eth_access_test.cc
)

set (sis3316ethB_tests_SRCS
sis3316ethB_tests.cc
)

# set everything needed for the root dictonary and create the
# dictionary
set(LINKDEF ${PROJECT_NAME}_LinkDef.h)
set(DICTIONARY ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Dict.cxx)
set(PCMFILE ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_rdict.pcm)
set(RMAPFILE ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.rootmap)
ROOT_GENERATE_DICTIONARY("${VMECards_HEADERS}" "${LINKDEF}" "${DICTIONARY}" "${INCLUDE_DIRECTORIES}" "${PROJECT_NAME}")

# add the dictionary to the list of source files
SET(VMECards_SRCS ${VMECards_HEADERS} ${VMECards_SRCS} ${DICTIONARY})

# Set the library version in the main CMakeLists.txt
SET(VMECards_MAJOR_VERSION 0)
SET(VMECards_MINOR_VERSION 0)
SET(VMECards_PATCH_VERSION 0)
SET(VMECards_VERSION "${VMECards_MAJOR_VERSION}.${VMECards_MINOR_VERSION}.${VMECards_PATCH_VERSION}")
SET(VMECards_LIBRARY_PROPERTIES ${VMECards_LIBRARY_PROPERTIES}
    VERSION "${VMECards_VERSION}"
    SOVERSION "${VMECards_MAJOR_VERSION}"
    SUFFIX ".so"
)

############### build the library #####################
add_library(VMECards SHARED ${VMECards_SRCS})
target_link_libraries(VMECards ${ROOT_LIBRARIES})
set_target_properties(VMECards PROPERTIES ${VMECards_LIBRARY_PROPERTIES})

add_executable(SIS3316SimpleTest ${SIS3316SimpleTest_SRCS})
target_link_libraries(SIS3316SimpleTest VMECards ${ROOT_LIBRARIES})


add_executable(SIS3350Scope ${SIS3350Scope_SRCS})
target_link_libraries(SIS3350Scope VMECards NGMData ${ROOT_LIBRARIES})

add_executable(SIS3316EthTest ${SIS3316EthTest_SRCS})
target_link_libraries(SIS3316EthTest VMECards NGMData ${ROOT_LIBRARIES})

add_executable(sis3316ethB_tests ${sis3316ethB_tests_SRCS})
target_link_libraries(sis3316ethB_tests VMECards ${ROOT_LIBRARIES})

############### install the library ###################
install(TARGETS SIS3316SimpleTest SIS3350Scope SIS3316EthTest DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(TARGETS sis3316ethB_tests DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(TARGETS VMECards DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${VMECards_HEADERS} DESTINATION include)
install(FILES ${PCMFILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${RMAPFILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

