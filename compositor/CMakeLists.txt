# Generate Wayland protocol headers required by wlroots
find_program(WAYLAND_SCANNER wayland-scanner REQUIRED)
pkg_get_variable(WAYLAND_PROTOCOLS_DIR wayland-protocols pkgdatadir)

set(XDG_SHELL_XML "${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml")
set(PROTO_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/protocol")
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

add_custom_command(
    OUTPUT ${PROTO_GEN_DIR}/xdg-shell-protocol.h
    COMMAND ${WAYLAND_SCANNER} server-header ${XDG_SHELL_XML}
            ${PROTO_GEN_DIR}/xdg-shell-protocol.h
    DEPENDS ${XDG_SHELL_XML}
    COMMENT "Generating xdg-shell-protocol.h"
)
add_custom_target(xdg-shell-protocol DEPENDS ${PROTO_GEN_DIR}/xdg-shell-protocol.h)

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
add_dependencies(lwindesk-compositor xdg-shell-protocol)

target_include_directories(lwindesk-compositor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PROTO_GEN_DIR}
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
