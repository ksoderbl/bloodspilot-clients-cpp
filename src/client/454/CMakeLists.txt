file(GLOB CLIENT_454_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

# The 454 client contains its own caudio.c, so exclude only when sound is off.
if(NOT BLOODSPILOT_ENABLE_SOUND)
    list(REMOVE_ITEM CLIENT_454_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/caudio.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/debugaudio.c"
    )
endif()

add_executable(bloodspilot-client-454 ${CLIENT_454_SOURCES})

target_include_directories(bloodspilot-client-454
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src/client
        ${PROJECT_BINARY_DIR}/generated
)

target_compile_definitions(bloodspilot-client-454
    PRIVATE
        XP454_CLIENT=1
        CONF_DATADIR="${BLOODSPILOT_CONF_DATADIR}"
        VERSION="${PROJECT_VERSION}"
)

target_link_libraries(bloodspilot-client-454
    PRIVATE
        X11::X11
        m
)

if(TARGET X11::Xext)
    target_link_libraries(bloodspilot-client-454 PRIVATE X11::Xext)
endif()

if(BLOODSPILOT_ENABLE_SOUND AND BLOODSPILOT_SOUND_LIBS)
    target_link_libraries(bloodspilot-client-454 PRIVATE ${BLOODSPILOT_SOUND_LIBS})
endif()

install(TARGETS bloodspilot-client-454
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
