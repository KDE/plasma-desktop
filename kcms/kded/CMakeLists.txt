# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm5_kded\")

add_library(kcm_kded MODULE kcmkded.cpp)
target_link_libraries(kcm_kded KF5::ConfigWidgets KF5::Service KF5::I18n Qt5::DBus)

install(TARGETS kcm_kded  DESTINATION ${KDE_INSTALL_PLUGINDIR} )

########### install files ###############

install( FILES kcmkded.desktop  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR} )
