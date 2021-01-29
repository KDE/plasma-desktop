add_definitions(-DTRANSLATION_DOMAIN=\"kcm_baloofile\")

set(kcm_file_SRCS
  kcm.cpp
  fileexcludefilters.cpp
  filteredfoldermodel.cpp
  baloodata.cpp
)

add_library(kcm_baloofile MODULE ${kcm_file_SRCS})

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

kcoreaddons_desktop_to_json(kcm_baloofile "kcm_baloofile.desktop")

install(FILES kcm_baloofile.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS kcm_baloofile DESTINATION ${KDE_INSTALL_PLUGINDIR}/kcms)

kpackage_install_package(package kcm_baloofile kcms)
