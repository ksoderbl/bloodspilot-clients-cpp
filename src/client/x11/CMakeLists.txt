file(GLOB X11_CLIENT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

add_executable(bloodspilot-client-x11 ${X11_CLIENT_SOURCES})

target_include_directories(bloodspilot-client-x11
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src/client
        ${PROJECT_BINARY_DIR}/generated
)

target_compile_definitions(bloodspilot-client-x11
    PRIVATE
        X11_CLIENT=1
        CONF_DATADIR="${BLOODSPILOT_CONF_DATADIR}"
        VERSION="${PROJECT_VERSION}"
)

target_link_libraries(bloodspilot-client-x11
    PRIVATE
        bloodspilot_client_common
        X11::X11
        m
)

if(TARGET X11::Xext)
    target_link_libraries(bloodspilot-client-x11 PRIVATE X11::Xext)
endif()

if(BLOODSPILOT_ENABLE_SOUND AND BLOODSPILOT_SOUND_LIBS)
    target_link_libraries(bloodspilot-client-x11 PRIVATE ${BLOODSPILOT_SOUND_LIBS})
endif()

install(TARGETS bloodspilot-client-x11
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
