# KI18N Translation Domain for this application
add_definitions(-DTRANSLATION_DOMAIN=\"kaccess\")


find_package(Phonon4Qt5 REQUIRED NO_MODULE)
include_directories(${PHONON_INCLUDE_DIR})

set(kaccess_KDEINIT_SRCS kaccess.cpp main.cpp )

kf5_add_kdeinit_executable( kaccess ${kaccess_KDEINIT_SRCS})

target_link_libraries(kdeinit_kaccess
    Qt5::Widgets
    Qt5::X11Extras
    KF5::ConfigCore
    KF5::Completion
    KF5::GlobalAccel
    KF5::I18n
    KF5::IconThemes
    KF5::Notifications
    KF5::WidgetsAddons
    KF5::WindowSystem
    Phonon::phonon4qt5
    KF5::KDELibs4Support
    ${X11_LIBRARIES}
)

install(TARGETS kdeinit_kaccess ${INSTALL_TARGETS_DEFAULT_ARGS} )
install(TARGETS kaccess         ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( FILES kaccess.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES kaccess.notifyrc  DESTINATION  ${KNOTIFYRC_INSTALL_DIR} )
