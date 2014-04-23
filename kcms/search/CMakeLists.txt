set(kcm_search_SRCS
  kcm.cpp
)

add_library(kcm_search MODULE ${kcm_search_SRCS})

target_link_libraries(kcm_search
  KF5::KIOWidgets
  KF5::CoreAddons
  KF5::KCMUtils
  KF5::Runner
  KF5::I18n

  Qt5::DBus
  Qt5::Widgets
)


install(FILES kcm_search.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS kcm_search DESTINATION ${PLUGIN_INSTALL_DIR})
