file(GLOB REPLAY_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

add_executable(bloodspilot-replay ${REPLAY_SOURCES})

target_include_directories(bloodspilot-replay
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src/client
        ${PROJECT_BINARY_DIR}/generated
)

target_compile_definitions(bloodspilot-replay
    PRIVATE
        HAVE_CONFIG_H=1
        REPLAY=1
        CONF_DATADIR="${BLOODSPILOT_CONF_DATADIR}"
        VERSION="${PROJECT_VERSION}"
)

target_link_libraries(bloodspilot-replay
    PRIVATE
        X11::X11
        m
)

install(TARGETS bloodspilot-replay
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
