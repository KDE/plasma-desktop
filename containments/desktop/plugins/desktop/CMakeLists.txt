set(desktopplugin_SRCS
    desktopplugin.cpp
    eventforge.cpp
)

install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/private/desktopcontainment/desktop)

add_library(desktopplugin SHARED ${desktopplugin_SRCS})

target_link_libraries(desktopplugin
                      Qt5::Core
                      Qt5::Qml
                      Qt5::Quick)

install(TARGETS desktopplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/private/desktopcontainment/desktop)
