cmake_minimum_required(VERSION 2.6)

project(NGMAnalysis)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

set( NGMAnalysis_SRCS
AnaAverageWaveform.cc
NGMPSDAnalyzer.cc
NGMRateMonitor.cc
NGMTimeMonitor.cc
NGMWaterfall.cc
NGMZoomGui.cc
NGMMarker.cc
NGMCanvas.cc
NGMDisplay.cc
NGMBurstMonitor.cc
NGMMultipleScattering.cc
)

set(NGMAnalysis_HEADERS
AnaAverageWaveform.h
NGMPSDAnalyzer.h
NGMRateMonitor.h
NGMTimeMonitor.h
NGMWaterfall.h
NGMZoomGui.h
NGMMarker.h
NGMCanvas.h
NGMDisplay.h
NGMBurstMonitor.h
NGMMultipleScattering.h
)

# set everything needed for the root dictonary and create the
# dictionary
set(LINKDEF ${PROJECT_NAME}_LinkDef.h)
set(DICTIONARY ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Dict.cxx)
set(PCMFILE ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_rdict.pcm)
set(RMAPFILE ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.rootmap)

ROOT_GENERATE_DICTIONARY("${NGMAnalysis_HEADERS}" "${LINKDEF}" "${DICTIONARY}" "${INCLUDE_DIRECTORIES}" "${PROJECT_NAME}")

# add the dictionary to the list of source files
SET(NGMAnalysis_SRCS ${NGMAnalysis_HEADERS} ${NGMAnalysis_SRCS} ${DICTIONARY})

# Set the library version in the main CMakeLists.txt
SET(NGMAnalysis_MAJOR_VERSION 0)
SET(NGMAnalysis_MINOR_VERSION 0)
SET(NGMAnalysis_PATCH_VERSION 0)
SET(NGMAnalysis_VERSION "${NGMAnalysis_MAJOR_VERSION}.${NGMAnalysis_MINOR_VERSION}.${NGMAnalysis_PATCH_VERSION}")
SET(NGMAnalysis_LIBRARY_PROPERTIES ${NGMAnalysis_LIBRARY_PROPERTIES}
    VERSION "${NGMAnalysis_VERSION}"
    SOVERSION "${NGMAnalysis_MAJOR_VERSION}"
    SUFFIX ".so"
)

############### build the library #####################
add_library(NGMAnalysis SHARED ${NGMAnalysis_SRCS})
target_link_libraries(NGMAnalysis NGMData NGMModuleBase NGMModuleCore VMECards ${ROOT_LIBRARIES} sqlite3 Spectrum Geom )
set_target_properties(NGMAnalysis PROPERTIES ${NGMAnalysis_LIBRARY_PROPERTIES})

############### install the library ###################
install(TARGETS NGMAnalysis DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${NGMAnalysis_HEADERS} DESTINATION include)
install(FILES ${PCMFILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${RMAPFILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

