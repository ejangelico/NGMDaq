cmake_minimum_required(VERSION 2.9)

project(SIS3302System)

#Check the compiler and set the compile and link flags
#set(CMAKE_BUILD_TYPE Debug)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

set( SIS3302System_SRCS
SIS3302System.cc
)

set(SIS3302System_HEADERS
SIS3302System.h
)

# set everything needed for the root dictionary and create the
# dictionary
set(LINKDEF ${PROJECT_NAME}_LinkDef.h)
set(DICTIONARY ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Dict.cxx)
set(PCMFILE ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_rdict.pcm)
set(RMAPFILE ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.rootmap)
ROOT_GENERATE_DICTIONARY("${SIS3302System_HEADERS}" "${LINKDEF}" "${DICTIONARY}" "${INCLUDE_DIRECTORIES};${PROJECT_SOURCE_DIR}" "${PROJECT_NAME}")

# add the dictionary to the list of source files
SET(SIS3302System_SRCS ${SIS3302System_HEADERS} ${SIS3302System_SRCS} ${DICTIONARY})

# Set the library version in the main CMakeLists.txt
SET(SIS3302System_MAJOR_VERSION 0)
SET(SIS3302System_MINOR_VERSION 0)
SET(SIS3302System_PATCH_VERSION 0)
SET(SIS3302System_VERSION "${SIS3302System_MAJOR_VERSION}.${SIS3302System_MINOR_VERSION}.${SIS3302System_PATCH_VERSION}")
SET(SIS3302System_LIBRARY_PROPERTIES ${SIS3302System_LIBRARY_PROPERTIES}
    VERSION "${SIS3302System_VERSION}"
    SOVERSION "${SIS3302System_MAJOR_VERSION}"
    SUFFIX ".so"
)

############### build the library #####################
add_library(SIS3302System SHARED ${SIS3302System_SRCS})
target_link_libraries(SIS3302System NGMModuleBase VMECards ${ROOT_LIBRARIES})
set_target_properties(SIS3302System PROPERTIES ${SIS3302System_LIBRARY_PROPERTIES})

#add_custom_command(TARGET SIS3302System POST_BUILD COMMAND rlibmap -o ${RMAPFILE} -l libSIS3302System -c ${CMAKE_CURRENT_SOURCE_DIR}/${SIS3302System_LINKDEF} COMMENT Install libNGMDaq.rootmap )

############### install the library ###################
install(TARGETS SIS3302System DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${SIS3302System_HEADERS} DESTINATION include)
install(FILES ${PCMFILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${RMAPFILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

