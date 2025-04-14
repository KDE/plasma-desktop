ecm_add_qml_module(kimpanelplugin
    URI "org.kde.plasma.private.kimpanel"
    GENERATE_PLUGIN_SOURCE
    VERSION 0.1
)

qt_add_dbus_adaptor(kimpanel_dbus_SRCS
    org.kde.impanel.xml
    kimpanelagent.h
    PanelAgent)

target_sources(kimpanelplugin PRIVATE
    kimpanel.cpp
    screen.cpp
    kimpanelagent.cpp
    ${kimpanel_dbus_SRCS}
)

target_compile_definitions(kimpanelplugin PRIVATE -DKIMPANEL_LIBEXEC_DIR=\"${KDE_INSTALL_FULL_LIBEXECDIR}\")
target_link_libraries(kimpanelplugin PRIVATE Qt::Gui Qt::Qml Qt::DBus)

ecm_finalize_qml_module(kimpanelplugin)
