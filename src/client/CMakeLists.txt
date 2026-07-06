file(GLOB CLIENT_COMMON_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

# These are only compiled when sound is enabled.
if(NOT BLOODSPILOT_ENABLE_SOUND)
    list(REMOVE_ITEM CLIENT_COMMON_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/caudio.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/oalaudio.c"
    )
endif()

add_library(bloodspilot_client_common STATIC ${CLIENT_COMMON_SOURCES})

target_include_directories(bloodspilot_client_common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${PROJECT_BINARY_DIR}/generated
)

target_compile_definitions(bloodspilot_client_common
    PUBLIC
        HAVE_CONFIG_H=1
        CONF_DATADIR="${BLOODSPILOT_CONF_DATADIR}"
)

target_link_libraries(bloodspilot_client_common PUBLIC m)

if(ZLIB_FOUND)
    target_link_libraries(bloodspilot_client_common PUBLIC ZLIB::ZLIB)
endif()

if(BLOODSPILOT_ENABLE_SOUND AND BLOODSPILOT_SOUND_LIBS)
    target_link_libraries(bloodspilot_client_common PUBLIC ${BLOODSPILOT_SOUND_LIBS})
endif()

if(BLOODSPILOT_BUILD_X11_CLIENT)
    add_subdirectory(x11)
endif()

if(BLOODSPILOT_BUILD_454_CLIENT)
    add_subdirectory(454)
endif()
