# KI18N Translation Domain for this application
add_definitions(-DTRANSLATION_DOMAIN=\"kaccess\")


find_package(Phonon4Qt5 REQUIRED NO_MODULE)
include_directories(${PHONON_INCLUDE_DIR})

set(kaccess_KDEINIT_SRCS kaccess.cpp main.cpp )

kf5_add_kdeinit_executable(kaccess ${kaccess_KDEINIT_SRCS})

target_link_libraries(kdeinit_kaccess
    Qt5::Widgets
    Qt5::X11Extras
    KF5::CoreAddons
    KF5::ConfigCore
    KF5::Completion
    KF5::GlobalAccel
    KF5::I18n
    KF5::Notifications
    KF5::WidgetsAddons
    KF5::WindowSystem
    KF5::DBusAddons
    Phonon::phonon4qt5
    ${X11_LIBRARIES}
)

install(TARGETS kdeinit_kaccess ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
install(TARGETS kaccess         ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( FILES kaccess.desktop  DESTINATION  ${KDE_INSTALL_AUTOSTARTDIR} )
install( FILES kaccess.notifyrc  DESTINATION  ${KDE_INSTALL_KNOTIFY5RCDIR} )
