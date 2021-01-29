# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_search\")

set(kcm_search_SRCS
  kcm.cpp
  krunnerdata.cpp
)

kconfig_add_kcfg_files(kcm_search_SRCS krunnersettings.kcfgc)

add_library(kcm_plasmasearch MODULE ${kcm_search_SRCS})
add_executable(plugininstaller
        plugininstaller/main.cpp
        plugininstaller/AbstractJob.cpp
        plugininstaller/ScriptJob.cpp
        plugininstaller/ZypperRPMJob.cpp)
if(HAVE_PACKAGEKIT)
    target_sources(plugininstaller PUBLIC plugininstaller/PackageKitJob.cpp)
endif()

set_target_properties(plugininstaller PROPERTIES
        OUTPUT_NAME "krunner-plugininstaller"
        )


target_link_libraries(kcm_plasmasearch
  KF5::KIOWidgets
  KF5::CoreAddons
  KF5::KCMUtils
  KF5::Runner
  KF5::I18n
  KF5::NewStuff
  KF5::Activities

  Qt::DBus
  Qt::Widgets
)

target_link_libraries(plugininstaller
  KF5::CoreAddons
  KF5::I18n
  Qt::Widgets
  KF5::Service
  KF5::KIOCore
  KF5::KIOWidgets
)
if(HAVE_PACKAGEKIT)
    target_link_libraries(plugininstaller PK::packagekitqt5)
endif()

install(FILES kcm_plasmasearch.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS kcm_plasmasearch DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES krunner.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
install(TARGETS plugininstaller DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
