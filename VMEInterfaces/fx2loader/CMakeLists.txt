cmake_minimum_required(VERSION 2.9)

project(fx2loaderlib)
set(CMAKE_SKIP_RPATH true)
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
)

link_directories( ${LINK_DIRECTORIES})

set( fx2loaderlib_SRCS
libusbwrap.c
ram.c
i2c.c
misc.c
misc_fx2.c
eeprom.c
conv.c
core.c
hexio.c
binio.c
liberror.c
fx2loadadaptor.c
)

set(fx2loaderlib_HEADERS
libusbwrap.h
libfx2loader.h
)

set( fx2loader_SRCS
argtable2.c
arg_end.c
arg_rem.c
arg_lit.c
arg_int.c
arg_dbl.c
arg_str.c
arg_file.c
arg_rex.c
arg_date.c
main.c
)


# Set the library version in the main CMakeLists.txt
SET(fx2loaderlib_MAJOR_VERSION 0)
SET(fx2loaderlib_MINOR_VERSION 0)
SET(fx2loaderlib_PATCH_VERSION 0)
SET(fx2loaderlib_VERSION "${fx2loaderlib_MAJOR_VERSION}.${fx2loaderlib_MINOR_VERSION}.${VMECards_PATCH_VERSION}")
SET(fx2loaderlib_LIBRARY_PROPERTIES ${fx2loaderlib_LIBRARY_PROPERTIES}
    VERSION "${fx2loaderlib_VERSION}"
    SOVERSION "${fx2loaderlib_MAJOR_VERSION}"
    SUFFIX ".so"
)

SET(fx2setuphexfile setup_8051.hex)
SET(fx2loaderscript SISPowerUp.command)

############### build the library #####################
add_library(fx2loaderlib SHARED ${fx2loaderlib_SRCS})
target_link_libraries(fx2loaderlib usb ${ROOT_LIBRARIES})
set_target_properties(fx2loaderlib PROPERTIES ${fx2loaderlib_LIBRARY_PROPERTIES})

add_executable(fx2loader ${fx2loader_SRCS})
target_link_libraries(fx2loader fx2loaderlib usb)

############### install the library ###################
install(TARGETS fx2loader DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(TARGETS fx2loaderlib DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${fx2setuphexfile} DESTINATION ${CMAKE_INSTALL_PREFIX}/etc )
install(FILES ${fx2loaderscript} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )

