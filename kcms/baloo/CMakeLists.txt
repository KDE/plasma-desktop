add_definitions(-DTRANSLATION_DOMAIN=\"kcm_baloofile\")

set(kcm_file_SRCS
  kcm.cpp
  fileexcludefilters.cpp
  filteredfoldermodel.cpp
  baloodata.cpp
)

kcoreaddons_add_plugin(kcm_baloofile SOURCES ${kcm_file_SRCS} INSTALL_NAMESPACE "plasma/kcms/systemsettings")

target_link_libraries(kcm_baloofile
  KF5::CoreAddons
  KF5::KCMUtils
  KF5::I18n
  KF5::Solid
  KF5::Baloo
  KF5::QuickAddons
  Qt::DBus
  Qt::Widgets
)

install(FILES kcm_baloofile.desktop DESTINATION ${KDE_INSTALL_APPDIR})

kpackage_install_package(package kcm_baloofile kcms)
