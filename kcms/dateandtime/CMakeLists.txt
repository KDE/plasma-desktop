# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmkclock\")
########### next target ###############

set(kcm_clock_PART_SRCS dtime.cpp main.cpp)

ki18n_wrap_ui(kcm_clock_PART_SRCS dateandtime.ui)

qt_add_dbus_interface(kcm_clock_PART_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/timedated1.xml timedated_interface)

add_library(kcm_clock MODULE ${kcm_clock_PART_SRCS})
kcmutils_generate_desktop_file(kcm_clock)

add_subdirectory( ui )

target_link_libraries(kcm_clock
    Qt::DBus
    Qt::Quick
    Qt::QuickWidgets
    KF6::ItemViews
    KF6::AuthCore
    KF6::GuiAddons
    KF6::KCMUtils
    KF6::I18n
    KF6::I18nLocaleData
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::Svg
    ICU::i18n
    ICU::uc
)
install(TARGETS kcm_clock  DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcms/systemsettings_qwidgets )

########### next target ###############
#This is only needed when not using timedated and can be removed in future

add_executable(kcmdatetimehelper helper.cpp ${helper_mocs})
target_link_libraries(kcmdatetimehelper KF6::AuthCore KF6::ConfigCore)

install(TARGETS kcmdatetimehelper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})
kauth_install_helper_files(kcmdatetimehelper org.kde.kcontrol.kcmclock root)

kauth_install_actions(org.kde.kcontrol.kcmclock kcmclock_actions.actions)

