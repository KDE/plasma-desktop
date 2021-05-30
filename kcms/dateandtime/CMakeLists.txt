# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmkclock\")
########### next target ###############

set(kcm_clock_PART_SRCS dtime.cpp main.cpp )

ki18n_wrap_ui(kcm_clock_PART_SRCS dateandtime.ui)

qt_add_dbus_interface(kcm_clock_PART_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/timedated1.xml timedated_interface)

add_library(kcm_clock MODULE ${kcm_clock_PART_SRCS})

target_link_libraries(kcm_clock
    Qt::DBus
    KF5::ItemViews
    KF5::AuthCore
    KF5::KCMUtils
    KF5::I18n
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::Plasma
    KF5::KDELibs4Support
)

install(TARGETS kcm_clock  DESTINATION ${KDE_INSTALL_PLUGINDIR} )

########### next target ###############
#This is only needed when not using timedated and can be removed in future

add_executable(kcmdatetimehelper helper.cpp ${helper_mocs})
target_link_libraries(kcmdatetimehelper KF5::AuthCore KF5::ConfigCore KF5::KDELibs4Support)

install(TARGETS kcmdatetimehelper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})
kauth_install_helper_files(kcmdatetimehelper org.kde.kcontrol.kcmclock root)

kauth_install_actions(org.kde.kcontrol.kcmclock kcmclock_actions.actions)

########### install files ###############

install( FILES clock.desktop  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR} )
