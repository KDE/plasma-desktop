set(desktopplugin_SRCS
    desktopplugin.cpp
    infonotification.cpp
)

install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/private/desktopcontainment/desktop)

add_library(desktopplugin SHARED ${desktopplugin_SRCS})

target_link_libraries(desktopplugin
                      Qt5::Core
                      Qt5::Qml
                      Qt5::Quick
                      Qt5::Widgets
                      KF5::Notifications)

install(TARGETS desktopplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/private/desktopcontainment/desktop)
