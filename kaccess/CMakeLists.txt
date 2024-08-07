# KI18N Translation Domain for this application
add_definitions(-DTRANSLATION_DOMAIN=\"kaccess\")

set(kaccess_SRCS kaccess.cpp main.cpp )

add_executable(kaccess ${kaccess_SRCS})

target_link_libraries(kaccess
    kaccess_settings
    Qt::Widgets
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::Completion
    KF6::GlobalAccel
    KF6::I18n
    KF6::Notifications
    KF6::WidgetsAddons
    KF6::WindowSystem
    KF6::DBusAddons
    KF6::Crash
    X11::X11
    Canberra::Canberra
)
target_link_libraries(kaccess Qt::GuiPrivate)

ecm_install_configured_files(
    INPUT plasma-kaccess.service.in
    @ONLY
    DESTINATION ${KDE_INSTALL_SYSTEMDUSERUNITDIR}
)

install(TARGETS kaccess ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES kaccess.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install( FILES kaccess.desktop  DESTINATION  ${KDE_INSTALL_AUTOSTARTDIR} )
install( FILES kaccess.notifyrc  DESTINATION  ${KDE_INSTALL_KNOTIFYRCDIR} )
