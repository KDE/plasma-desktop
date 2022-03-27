# KI18N Translation Domain for this application
add_definitions(-DTRANSLATION_DOMAIN=\"kaccess\")

find_package(Phonon4Qt${QT_MAJOR_VERSION} REQUIRED NO_MODULE)

set(kaccess_SRCS kaccess.cpp main.cpp )

add_executable(kaccess ${kaccess_SRCS})

target_link_libraries(kaccess
    Qt::Widgets
    KF5::CoreAddons
    KF5::ConfigCore
    KF5::Completion
    KF5::GlobalAccel
    KF5::I18n
    KF5::Notifications
    KF5::WidgetsAddons
    KF5::WindowSystem
    KF5::DBusAddons
    Phonon::phonon4qt${QT_MAJOR_VERSION}
    X11::X11
)
if (QT_MAJOR_VERSION EQUAL "5")
    target_link_libraries(kaccess Qt::X11Extras)
endif()

install(TARGETS kaccess ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES kaccess.desktop  DESTINATION  ${KDE_INSTALL_AUTOSTARTDIR} )
install( FILES kaccess.notifyrc  DESTINATION  ${KDE_INSTALL_KNOTIFYRCDIR} )
