add_executable(lwindesk-compositor
    src/main.c
    src/server.c
    src/output.c
    src/view.c
    src/input.c
    src/xdg_shell.c
    src/layer_shell.c
    src/workspace.c
    src/snap.c
)

target_include_directories(lwindesk-compositor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${WLROOTS_INCLUDE_DIRS}
    ${WAYLAND_SERVER_INCLUDE_DIRS}
    ${XKBCOMMON_INCLUDE_DIRS}
    ${LIBINPUT_INCLUDE_DIRS}
    ${PIXMAN_INCLUDE_DIRS}
    ${DRM_INCLUDE_DIRS}
    ${GBM_INCLUDE_DIRS}
    ${LIBSEAT_INCLUDE_DIRS}
    ${SYSTEMD_INCLUDE_DIRS}
)

target_compile_definitions(lwindesk-compositor PRIVATE
    WLR_USE_UNSTABLE
    _POSIX_C_SOURCE=200112L
)

target_link_libraries(lwindesk-compositor PRIVATE
    ${WLROOTS_LIBRARIES}
    ${WAYLAND_SERVER_LIBRARIES}
    ${XKBCOMMON_LIBRARIES}
    ${LIBINPUT_LIBRARIES}
    ${PIXMAN_LIBRARIES}
    ${DRM_LIBRARIES}
    ${GBM_LIBRARIES}
    ${LIBSEAT_LIBRARIES}
    ${SYSTEMD_LIBRARIES}
    m
)

install(TARGETS lwindesk-compositor DESTINATION bin)
