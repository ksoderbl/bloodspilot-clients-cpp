file(GLOB XPCLIENT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# These are only compiled when sound is enabled.
if(NOT BLOODSPILOT_ENABLE_SOUND)
    list(REMOVE_ITEM XPCLIENT_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/caudio.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/oalaudio.cpp"
    )
endif()

add_library(xpclient STATIC ${XPCLIENT_SOURCES})

target_include_directories(xpclient
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/common
        ${CMAKE_BINARY_DIR}/generated
)

target_compile_definitions(xpclient
    PUBLIC
        HAVE_CONFIG_H=1
        CONF_DATADIR="${BLOODSPILOT_CONF_DATADIR}"
)

target_link_libraries(xpclient PUBLIC m)

if(ZLIB_FOUND)
    target_link_libraries(xpclient PUBLIC ZLIB::ZLIB)
endif()

if(BLOODSPILOT_ENABLE_SOUND AND BLOODSPILOT_SOUND_LIBS)
    target_link_libraries(xpclient PUBLIC ${BLOODSPILOT_SOUND_LIBS})
endif()

if(BLOODSPILOT_BUILD_X11_CLIENT)
    add_subdirectory(x11)
endif()
