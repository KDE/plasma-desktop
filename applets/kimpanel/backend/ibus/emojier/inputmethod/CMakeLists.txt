ecm_add_qtwayland_client_protocol(SRCS
    PROTOCOL ${WaylandProtocols_DATADIR}/unstable/input-method/input-method-unstable-v1.xml
    BASENAME input-method-unstable-v1
)
ecm_add_qtwayland_client_protocol(SRCS
    PROTOCOL ${WaylandProtocols_DATADIR}/unstable/text-input/text-input-unstable-v1.xml
    BASENAME text-input-unstable-v1
)

add_executable(ibus-ui-emojier-inputmethod main.cpp inputmethod.cpp inputbracket.cpp resources.qrc ${SRCS})
target_link_libraries(ibus-ui-emojier-inputmethod PUBLIC Qt5::Quick KF5::WaylandClient Wayland::Client)

install(TARGETS ibus-ui-emojier-inputmethod DESTINATION bin)
install(FILES org.kde.plasma.emojier.inputmethod.desktop DESTINATION share/applications)
