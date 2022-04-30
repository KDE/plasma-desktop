add_library(kcm_plasmasearch MODULE kcm.cpp krunnerdata.cpp)

target_link_libraries(kcm_plasmasearch
  KF5::CoreAddons
  KF5::KCMUtils
  KF5::Runner
  KF5::I18n
  KF5::NewStuffWidgets

  Qt::DBus
  Qt::Widgets
)

install(TARGETS kcm_plasmasearch DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcms/systemsettings_qwidgets)
install(FILES kcm_plasmasearch.desktop DESTINATION ${KDE_INSTALL_APPDIR})
