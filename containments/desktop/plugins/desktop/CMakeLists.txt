set(desktopplugin_SRCS
    desktopplugin.cpp
    eventforge.cpp
    infonotification.cpp
    systemsettings.cpp
)

install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/private/desktopcontainment/desktop)

add_library(desktopplugin SHARED ${desktopplugin_SRCS})

target_link_libraries(desktopplugin
                      Qt5::Core
                      Qt5::Qml
                      Qt5::Quick
                      Qt5::Widgets
                      KF5::Notifications)

install(TARGETS desktopplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/private/desktopcontainment/desktop)
