set(desktopplugin_SRCS
    desktopplugin.cpp
    infonotification.cpp
)

install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/private/desktopcontainment/desktop)

add_library(desktopplugin SHARED ${desktopplugin_SRCS})

target_link_libraries(desktopplugin
                      Qt::Core
                      Qt::Qml
                      Qt::Quick
                      Qt::Widgets
                      KF5::Notifications)

install(TARGETS desktopplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/private/desktopcontainment/desktop)
