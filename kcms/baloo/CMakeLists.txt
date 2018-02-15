add_definitions(-DTRANSLATION_DOMAIN=\"kcm5_baloofile\")

set(kcm_file_SRCS
  kcm.cpp
  fileexcludefilters.cpp
  folderselectionwidget.cpp
)

ki18n_wrap_ui(kcm_file_SRCS
  configwidget.ui
)

add_library(kcm_baloofile MODULE ${kcm_file_SRCS})

target_link_libraries(kcm_baloofile
  KF5::KIOWidgets
  KF5::CoreAddons
  KF5::KCMUtils
  KF5::I18n
  KF5::Solid
  KF5::Baloo

  Qt5::DBus
  Qt5::Widgets
)


install(FILES kcm_baloofile.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS kcm_baloofile DESTINATION ${KDE_INSTALL_PLUGINDIR})
