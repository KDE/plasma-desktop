add_definitions(-DTRANSLATION_DOMAIN=\"kcm_baloofile\")

set(kcm_file_SRCS
  kcm.cpp
  fileexcludefilters.cpp
  filteredfoldermodel.cpp
  baloodata.cpp
)

kcoreaddons_add_plugin(kcm_baloofile SOURCES ${kcm_file_SRCS} INSTALL_NAMESPACE "plasma/kcms/systemsettings")
kcmutils_generate_desktop_file(kcm_baloofile)

target_link_libraries(kcm_baloofile
  KF6::CoreAddons
  KF6::KCMUtils
  KF6::I18n
  KF6::Solid
  KF6::Baloo
  KF6::KCMUtilsQuick
  Qt::DBus
  Qt::Widgets
)

kpackage_install_package(package kcm_baloofile kcms)
