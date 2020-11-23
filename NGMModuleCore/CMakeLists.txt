cmake_minimum_required(VERSION 2.6)

project(NGMModuleCore)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

set( NGMModuleCore_SRCS
NGMBaseline.cc
NGMReaderBase.cc
NGMPIXIE16RawReaderv2.cc
PIXIE16RawReaderv3.cc
PIXIE16RawReaderv4.cc
NGMSIS3316RawReader.cc
NGMSIS3302RawReader.cc
NGMMultiFormatReader.cc
NGMPacketBufferIO.cc
NGMHitIO.cc
NGMHitProcess.cc
NGMHitPSD.cc
NGMPacketMergeSort.cc
NGMPSDMaker.cc
NGMTimeGapFinder.cc
NGMParticleIdent.cc
NGMSimpleParticleIdent.cc
NGMTof.cc
NGMTimingCal.cc
)

set(NGMModuleCore_HEADERS
NGMBaseline.h
NGMReaderBase.h
NGMSIS3316RawReader.h
NGMSIS3302RawReader.h
NGMPIXIE16RawReaderv2.h
PIXIE16RawReaderv3.h
PIXIE16RawReaderv4.h
NGMMultiFormatReader.h
NGMPacketBufferIO.h
NGMHitIO.h
NGMHitProcess.h
NGMHitPSD.h
NGMPacketMergeSort.h
NGMPSDMaker.h
NGMTimeGapFinder.h
NGMParticleIdent.h
NGMSimpleParticleIdent.h
NGMTof.h
NGMTimingCal.h
)

# set everything needed for the root dictonary and create the
# dictionary
set(LINKDEF ${PROJECT_NAME}_LinkDef.h)
set(DICTIONARY ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Dict.cxx)
set(PCMFILE ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_rdict.pcm)
set(RMAPFILE ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.rootmap)
ROOT_GENERATE_DICTIONARY("${NGMModuleCore_HEADERS}" "${LINKDEF}" "${DICTIONARY}" "${INCLUDE_DIRECTORIES}" "${PROJECT_NAME}")

# add the dictionary to the list of source files
SET(NGMModuleCore_SRCS ${NGMModuleCore_HEADERS} ${NGMModuleCore_SRCS} ${DICTIONARY})

# Set the library version in the main CMakeLists.txt
SET(NGMModuleCore_MAJOR_VERSION 0)
SET(NGMModuleCore_MINOR_VERSION 0)
SET(NGMModuleCore_PATCH_VERSION 0)
SET(NGMModuleCore_VERSION "${NGMModuleCore_MAJOR_VERSION}.${NGMModuleCore_MINOR_VERSION}.${NGMModuleCore_PATCH_VERSION}")
SET(NGMModuleCore_LIBRARY_PROPERTIES ${NGMModuleCore_LIBRARY_PROPERTIES}
    VERSION "${NGMModuleCore_VERSION}"
    SOVERSION "${NGMModuleCore_MAJOR_VERSION}"
    SUFFIX ".so"
)

############### build the library #####################
add_library(NGMModuleCore SHARED ${NGMModuleCore_SRCS})
target_link_libraries(NGMModuleCore NGMData NGMModuleBase VMECards ${ROOT_LIBRARIES} Spectrum )
set_target_properties(NGMModuleCore PROPERTIES ${NGMModuleCore_LIBRARY_PROPERTIES})

############### install the library ###################
install(TARGETS NGMModuleCore DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${NGMModuleCore_HEADERS} DESTINATION include)
install(FILES ${PCMFILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${RMAPFILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

