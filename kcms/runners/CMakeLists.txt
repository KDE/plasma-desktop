# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_search\")

set(kcm_search_SRCS
  kcm.cpp
)

add_library(kcm_plasmasearch MODULE ${kcm_search_SRCS})

target_link_libraries(kcm_plasmasearch
  KF5::KIOWidgets
  KF5::CoreAddons
  KF5::KCMUtils
  KF5::Runner
  KF5::I18n

  Qt5::DBus
  Qt5::Widgets
)


install(FILES kcm_plasmasearch.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS kcm_plasmasearch DESTINATION ${KDE_INSTALL_PLUGINDIR})
