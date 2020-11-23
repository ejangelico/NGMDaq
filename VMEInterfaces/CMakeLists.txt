cmake_minimum_required(VERSION 2.6)

project(VMEInterfaces)
#set(CMAKE_SKIP_RPATH true)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

include_directories(${CMAKE_INSTALL_PREFIX}/include)

include_directories(/usr/local/include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Dunix " )
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Dunix " )

link_directories( ${LINK_DIRECTORIES} ${CMAKE_INSTALL_PREFIX}/lib /usr/local/lib )

set( VMEInterfaces_SRCS
sis3150usb_utils.c
sis3150usb_vme_calls.c
sis3150usb_vme_linux.c
sis3150_vmeinterface.cc
)

set(VMEInterfaces_HEADERS
sis3150_vmeinterface.h
)

# set everything needed for the root dictonary and create the
# dictionary
set(LINKDEF ${PROJECT_NAME}_LinkDef.h)
set(DICTIONARY ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Dict.cxx)
set(PCMFILE ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_rdict.pcm)
set(RMAPFILE ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.rootmap)
ROOT_GENERATE_DICTIONARY("${VMEInterfaces_HEADERS}" "${LINKDEF}" "${DICTIONARY}" "${INCLUDE_DIRECTORIES}" "${PROJECT_NAME}")

# add the dictionary to the list of source files
SET(VMEInterfaces_SRCS ${VMEInterfaces_HEADERS} ${VMEInterfaces_SRCS} ${DICTIONARY})

# Set the library version in the main CMakeLists.txt
SET(VMEInterfaces_MAJOR_VERSION 0)
SET(VMEInterfaces_MINOR_VERSION 0)
SET(VMEInterfaces_PATCH_VERSION 0)
SET(VMEInterfaces_VERSION "${VMEInterfaces_MAJOR_VERSION}.${VMEInterfaces_MINOR_VERSION}.${VMEInterfaces_PATCH_VERSION}")
SET(VMEInterfaces_LIBRARY_PROPERTIES ${VMEInterfaces_LIBRARY_PROPERTIES}
    VERSION "${VMEInterfaces_VERSION}"
    SOVERSION "${VMEInterfaces_MAJOR_VERSION}"
    SUFFIX ".so"
)

############### build the library #####################
add_library(VMEInterfaces SHARED ${VMEInterfaces_SRCS})
target_link_libraries(VMEInterfaces VMECards usb ${ROOT_LIBRARIES})
set_target_properties(VMEInterfaces PROPERTIES ${VMEInterfaces_LIBRARY_PROPERTIES})

############### install the library ###################
install(TARGETS VMEInterfaces DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${PCMFILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${RMAPFILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

add_subdirectory(fx2loader)
