cmake_minimum_required(VERSION 3.10)

project(glfer VERSION 0.5.0)
set(PACKAGE_NAME "glfer")
set(PACKAGE_VERSION ${VERSION})
add_definitions(-DPACKAGE_NAME="${PACKAGE_NAME}")
add_definitions(-DPACKAGE_VERSION="${PACKAGE_VERSION}")
add_definitions(-DPACKAGE_STRING="${PACKAGE_NAME} ${PACKAGE_VERSION}")

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(deps REQUIRED IMPORTED_TARGET glib-2.0>=2.8)

# GTK2
pkg_check_modules(GTK2 REQUIRED gtk+-2.0)
include_directories(${GTK2_INCLUDE_DIRS})
link_directories(${GTK2_LIBRARY_DIRS})
add_definitions(${GTK2_CFLAGS_OTHER})

# sndfile
pkg_check_modules(SNDFILE REQUIRED sndfile)
include_directories(${SNDFILE_INCLUDE_DIRS})
link_directories(${SNDFILE_LIBRARY_DIRS})
add_definitions(${SNDFILE_CFLAGS_OTHER})

# PulseAudio (for PipeWire compatibility)
pkg_check_modules(PULSE REQUIRED libpulse-simple)
include_directories(${PULSE_INCLUDE_DIRS})
link_directories(${PULSE_LIBRARY_DIRS})
add_definitions(${PULSE_CFLAGS_OTHER})

# FFTW2
find_path(FFTW2_INCLUDE_DIR fftw.h)
find_library(FFTW2_LIBRARY fftw)
find_library(RFFTW2_LIBRARY rfftw)

if(NOT FFTW2_INCLUDE_DIR OR NOT FFTW2_LIBRARY OR NOT RFFTW2_LIBRARY)
    message(FATAL_ERROR "FFTW2 library not found")
endif()
include_directories(${FFTW2_INCLUDE_DIR})

# Set sources for glfer
set(GLFER_SOURCES
    glfer.c
    glfer.h
    fft.c
    fft.h
    audio.c
    audio.h
    avg.c
    avg.h
    cw_rx.c
    cw_rx.h
    util.c
    util.h
    qrs.c
    qrs.h
    rcfile.c
    rcfile.h
    source.c
    source.h
    sndread.c
    sndread.h
    mtm.c
    mtm.h
    lmp.c
    lmp.h
    bell-p-w.c
    bell-p-w.h
    g-l_dpss.c
    g-l_dpss.h
    hparma.c
    hparma.h
    g_main.c
    g_main.h
    g_about.c
    g_about.h
    g_txmsg.c
    g_txmsg.h
    g_options.c
    g_options.h
    g_scope.c
    g_scope.h
    g_file_dialogs.c
    g_file_dialogs.h
)

# Add glfer executable
add_executable(glfer ${GLFER_SOURCES})
target_link_libraries(glfer -lm PkgConfig::deps ${GTK2_LIBRARIES} ${PULSE_LIBRARIES} ${SNDFILE_LIBRARIES} ${RFFTW2_LIBRARY} ${FFTW2_LIBRARY})

# Add mixer executable
add_executable(mixer mixer.c)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/app_ver.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/app_ver.h
    @ONLY
)
target_include_directories(glfer PRIVATE ${CMAKE_BINARY_DIR})

# Install targets
install(TARGETS glfer mixer RUNTIME DESTINATION bin)

# Install man pages
install(FILES glfer.1 DESTINATION share/man/man1)

# Extra files
set(EXTRA_DIST HACKING TODO glfer.lsm glfer.1)
install(FILES ${EXTRA_DIST} DESTINATION share/doc/glfer)

