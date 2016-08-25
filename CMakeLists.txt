cmake_minimum_required(VERSION 2.8)
project(epoller)

set(EPOLLER_VERSION_MAJOR 1)
set(EPOLLER_VERSION_MINOR 1)

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(GLIB glib-2.0)
endif(PKG_CONFIG_FOUND)

set(SOURCES_EPOLLER
    src/epoller/epoller.cpp
    src/epoller/evepoller.cpp
    src/epoller/fdepoller.cpp
    src/epoller/gpioepoller.cpp
    src/epoller/inotepoller.cpp
    src/epoller/mntepoller.cpp
    src/epoller/sigepoller.cpp
    src/epoller/sockepoller.cpp
    src/epoller/timepoller.cpp
    src/epoller/ttyepoller.cpp)

set(SOURCES_LINBUFF
    src/linbuff/linbuff.c)

set(HEADERS_EPOLLER
    include/epoller/epoller.h
    include/epoller/evepoller.h
    include/epoller/fdepoller.h
    include/epoller/gpioepoller.h
    include/epoller/inotepoller.h
    include/epoller/mntepoller.h
    include/epoller/sigepoller.h
    include/epoller/sockepoller.h
    include/epoller/timepoller.h
    include/epoller/ttyepoller.h)

set(HEADERS_LINBUFF
    include/linbuff/linbuff.h)

if(GLIB_FOUND)
    set(SOURCES_EPOLLER ${SOURCES_EPOLLER} src/epoller/gepoller.cpp)
    set(HEADERS_EPOLLER ${HEADERS_EPOLLER} include/epoller/gepoller.h)
endif(GLIB_FOUND)

include_directories(include)
add_library(epoller SHARED ${SOURCES_EPOLLER} ${SOURCES_LINBUFF})

if(GLIB_FOUND)
    include_directories(${GLIB_INCLUDE_DIRS})
    target_link_libraries(epoller ${GLIB_LIBRARIES})
endif(GLIB_FOUND)

install(TARGETS epoller DESTINATION lib)
install(FILES ${HEADERS_EPOLLER} DESTINATION include/epoller)
install(FILES ${HEADERS_LINBUFF} DESTINATION include/linbuff)

if(PKG_CONFIG_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig/epoller.pc.in
                   ${CMAKE_CURRENT_BINARY_DIR}/pkgconfig/epoller.pc @ONLY)
    install(FILES  ${CMAKE_CURRENT_BINARY_DIR}/pkgconfig/epoller.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
endif(PKG_CONFIG_FOUND)

find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_INPUT                  "${CMAKE_CURRENT_SOURCE_DIR}/include/epoller")
    set(DOXYGEN_INPUT "${DOXYGEN_INPUT} ${CMAKE_CURRENT_SOURCE_DIR}/include/linbuff")
    set(DOXYGEN_INPUT "${DOXYGEN_INPUT} ${CMAKE_CURRENT_SOURCE_DIR}/src/epoller"    )
    set(DOXYGEN_INPUT "${DOXYGEN_INPUT} ${CMAKE_CURRENT_SOURCE_DIR}/src/linbuff"    )
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doxy/Doxyfile.in
                   ${CMAKE_CURRENT_BINARY_DIR}/doxy/Doxyfile @ONLY)
    add_custom_target(doc
                      COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxy/Doxyfile
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                      COMMENT "Generating documentation" VERBATIM)
endif(DOXYGEN_FOUND)

