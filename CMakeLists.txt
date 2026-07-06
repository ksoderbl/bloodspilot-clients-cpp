cmake_minimum_required(VERSION 3.16)

project(bloodspilot-clients VERSION 1.4.0 LANGUAGES C)

include(GNUInstallDirs)
include(CheckFunctionExists)

option(BLOODSPILOT_BUILD_X11_CLIENT "Build the X11 client" ON)
option(BLOODSPILOT_BUILD_454_CLIENT "Build the 4.5.4 style client" ON)
option(BLOODSPILOT_BUILD_REPLAY "Build the replay program" ON)
option(BLOODSPILOT_ENABLE_SOUND "Enable OpenAL sound support" OFF)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Package-like values similar to autotools/config.h
set(PACKAGE_NAME "BloodsPilot")
set(PACKAGE_TARNAME "bloodspilot-clients")
set(PACKAGE_VERSION "${PROJECT_VERSION}")
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "https://github.com/ksoderbl/bloodspilot-clients/issues")
set(PACKAGE_URL "https://github.com/ksoderbl/bloodspilot-clients")
set(VERSION "${PROJECT_VERSION}")

# Match autotools pkgdatadir style:
# $(pkgdatadir) == ${datarootdir}/bloodspilot-clients
set(BLOODSPILOT_PKGDATADIR "${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME}")
set(BLOODSPILOT_CONF_DATADIR "${BLOODSPILOT_PKGDATADIR}/")

check_function_exists(strlcpy HAVE_STRLCPY)
check_function_exists(strlcat HAVE_STRLCAT)
check_function_exists(gettimeofday HAVE_GETTIMEOFDAY)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/config.h
    @ONLY
)

find_package(X11 REQUIRED)
find_package(ZLIB QUIET)
find_package(OpenGL QUIET)

if(ZLIB_FOUND)
    set(HAVE_ZLIB_H 1)
endif()

set(BLOODSPILOT_SOUND_LIBS "")
if(BLOODSPILOT_ENABLE_SOUND)
    find_library(OPENAL_LIBRARY openal)
    find_library(ALUT_LIBRARY alut)

    if(OPENAL_LIBRARY AND ALUT_LIBRARY)
        set(BLOODSPILOT_SOUND_LIBS ${OPENAL_LIBRARY} ${ALUT_LIBRARY})
        add_compile_definitions(SOUND=1)
    else()
        message(WARNING "OpenAL/ALUT not found, disabling sound")
    endif()
endif()

# add_compile_options(
#     -Wall
#     -Wextra
#     -Wpedantic
#     -Wno-missing-field-initializers
#     -Wno-unused-parameter
# )

add_compile_options(
    -Wall
    -Wextra
    -Wformat=2
    -Wundef
    -Wshadow
    -Wpointer-arith
    -Wcast-qual
    -Wwrite-strings
    -Wmissing-declarations
    -Wuninitialized
    -Winit-self
    -Wredundant-decls
    -Wstrict-prototypes
    -Wmissing-prototypes
    -Wold-style-definition
    -Wno-unused-parameter
)

add_subdirectory(src)

# Install runtime data like autotools share/Makefile.am
install(
    FILES share/shipshapes.txt
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}
)

install(
    DIRECTORY share/textures/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/textures
)

if(BLOODSPILOT_ENABLE_SOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/share/sound")
    install(
        DIRECTORY share/sound/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/sound
    )
endif()
