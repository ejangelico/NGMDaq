cmake_minimum_required(VERSION 2.6)

project(EXOUtilities)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

set( EXOUtilities_SRCS
EXOErrorLogger.cc
EXOTemplWaveform.cc
EXOVWaveformTransformer.cc
EXOBaselineRemover.cc
EXOSmoother.cc
EXOPoleZeroCorrection.cc
EXORisetimeCalculation.cc
EXOExtremumFinder.cc
EXOTrapezoidalFilter.cc
EXODecayTimeFit.cc
EXOWaveformFT.cc
EXOMatchedFilter.cc
EXOFastFourierTransformFFTW.cc
)

set(EXOUtilities_HEADERS
SystemOfUnits.hh
EXOMiscUtil.hh
EXOErrorLogger.hh
EXOTemplWaveform.hh
EXOVWaveformTransformer.hh
EXOBaselineRemover.hh
EXOSmoother.hh
EXOPoleZeroCorrection.hh
EXORisetimeCalculation.hh
EXOExtremumFinder.hh
EXOTrapezoidalFilter.hh
EXODecayTimeFit.hh
EXOWaveformFT.hh
EXOMatchedFilter.hh
EXOFastFourierTransformFFTW.hh
)

# set everything needed for the root dictonary and create the
# dictionary
set(LINKDEF ${PROJECT_NAME}_LinkDef.h)
set(DICTIONARY ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Dict.cxx)
set(PCMFILE ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_rdict.pcm)
set(RMAPFILE ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.rootmap)
ROOT_GENERATE_DICTIONARY("${EXOUtilities_HEADERS}" "${LINKDEF}" "${DICTIONARY}" "${INCLUDE_DIRECTORIES}" "${PROJECT_NAME}")

# add the dictionary to the list of source files
SET(EXOUtilities_SRCS ${EXOUtilities_HEADERS} ${EXOUtilities_SRCS} ${DICTIONARY})

# Set the library version in the main CMakeLists.txt
SET(EXOUtilities_MAJOR_VERSION 0)
SET(EXOUtilities_MINOR_VERSION 0)
SET(EXOUtilities_PATCH_VERSION 0)
SET(EXOUtilities_VERSION "${EXOUtilities_MAJOR_VERSION}.${EXOUtilities_MINOR_VERSION}.${EXOUtilities_PATCH_VERSION}")
SET(EXOUtilities_LIBRARY_PROPERTIES ${EXOUtilities_LIBRARY_PROPERTIES}
    VERSION "${EXOUtilities_VERSION}"
    SOVERSION "${EXOUtilities_MAJOR_VERSION}"
    SUFFIX ".so"
)


############### build the library #####################
add_library(EXOUtilities SHARED ${EXOUtilities_SRCS})
target_link_libraries(EXOUtilities ${ROOT_LIBRARIES} -lFFTW -lfftw3)
set_target_properties(EXOUtilities PROPERTIES ${EXOUtilities_LIBRARY_PROPERTIES})

set(INCLUDE_FLAGS)
foreach (_current_FILE ${INCLUDE_DIRECTORIES})
  set(INCLUDE_FLAGS ${INCLUDE_FLAGS} -I${_current_FILE})
endforeach (_current_FILE ${INCLUDE_DIRECTORIES})

############### install the library ###################
install(TARGETS EXOUtilities DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${EXOUtilities_HEADERS} DESTINATION include)
install(FILES ${PCMFILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${RMAPFILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

