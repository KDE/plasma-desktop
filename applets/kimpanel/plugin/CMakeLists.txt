set(kimpanelplugin_SRCS
    kimpanel.cpp
    screen.cpp
    kimpanelplugin.cpp
    kimpanelagent.cpp
)
QT5_ADD_DBUS_ADAPTOR(kimpanelplugin_SRCS
    org.kde.impanel.xml
    kimpanelagent.h
    PanelAgent)

add_library(kimpanelplugin SHARED ${kimpanelplugin_SRCS})
target_compile_definitions(kimpanelplugin PRIVATE -DKIMPANEL_LIBEXEC_DIR=\"${KDE_INSTALL_FULL_LIBEXECDIR}\")
target_link_libraries(kimpanelplugin Qt::Gui Qt::Qml Qt::DBus)
install(TARGETS kimpanelplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/kimpanel)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/kimpanel)
