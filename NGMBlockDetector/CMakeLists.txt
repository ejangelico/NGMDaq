cmake_minimum_required(VERSION 2.9)

project(NGMBlockDetector)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

set( NGMBlockDetector_SRCS
CoincidenceImager.cc
CoincidenceMaker.cc
NGMBlockAliveMonitor.cc
NGMBlockArrayMonitor.cc
NGMBlockDetectorCalibrator.cc
NGMBlockFlood.cc
NGMBlockMapping.cc
NGMBlockPSDMaker.cc
NGMPixelADCMonitor.cc
NGMBlockPicoDST.cc
ImageMaker.cc
LinearScanAnalysis.cc
DepthScan.cc
MyArray2d.cc
InteractiveDecoder.cc
PDSTImager.cc
NGMSISCalibrator.cc
NGMBlockMultipleScattering.cc
NGMBlockFNWellCounter.cc
)

set(NGMBlockDetector_HEADERS
CoincidenceImager.h
CoincidenceMaker.h
NGMBlockAliveMonitor.h
NGMBlockArrayMonitor.h
NGMBlockDetectorCalibrator.h
NGMBlockFlood.h
NGMBlockMapping.h
NGMBlockPSDMaker.h
NGMPixelADCMonitor.h
NGMBlockPicoDST.h
ImageMaker.h
LinearScanAnalysis.h
DepthScan.h
MyArray2d.h
PDSTImager.h
InteractiveDecoder.h
NGMBlockMultipleScattering.h
NGMSISCalibrator.h
NGMBlockFNWellCounter.h
)

# set everything needed for the root dictonary and create the
# dictionary
set(LINKDEF ${PROJECT_NAME}_LinkDef.h)
set(DICTIONARY ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Dict.cxx)
set(PCMFILE ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_rdict.pcm)
set(RMAPFILE ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.rootmap)
ROOT_GENERATE_DICTIONARY("${NGMBlockDetector_HEADERS}" "${LINKDEF}" "${DICTIONARY}" "${INCLUDE_DIRECTORIES}" "${PROJECT_NAME}")

# add the dictionary to the list of source files
SET(NGMBlockDetector_SRCS ${NGMBlockDetector_HEADERS} ${NGMBlockDetector_SRCS} ${DICTIONARY})

# Set the library version in the main CMakeLists.txt
SET(NGMBlockDetector_MAJOR_VERSION 0)
SET(NGMBlockDetector_MINOR_VERSION 0)
SET(NGMBlockDetector_PATCH_VERSION 0)
SET(NGMBlockDetector_VERSION "${NGMBlockDetector_MAJOR_VERSION}.${NGMBlockDetector_MINOR_VERSION}.${NGMBlockDetector_PATCH_VERSION}")
SET(NGMBlockDetector_LIBRARY_PROPERTIES ${NGMBlockDetector_LIBRARY_PROPERTIES}
    VERSION "${NGMBlockDetector_VERSION}"
    SOVERSION "${NGMBlockDetector_MAJOR_VERSION}"
    SUFFIX ".so"
)

############### build the library #####################
add_library(NGMBlockDetector SHARED ${NGMBlockDetector_SRCS})
target_link_libraries(NGMBlockDetector NGMData NGMModuleBase NGMModuleCore ${ROOT_LIBRARIES} Proof)
set_target_properties(NGMBlockDetector PROPERTIES ${NGMBlockDetector_LIBRARY_PROPERTIES})

############### install the library ###################
install(TARGETS NGMBlockDetector DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${NGMBlockDetector_HEADERS} DESTINATION include)
install(FILES ${PCMFILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${RMAPFILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

