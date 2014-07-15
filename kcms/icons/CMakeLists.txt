# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmicons\")

add_subdirectory(tests)

########### next target ###############

set(kcm_icons_PART_SRCS iconthemes.cpp icons.cpp main.cpp )

add_library(kcm_icons MODULE ${kcm_icons_PART_SRCS})

target_link_libraries(kcm_icons
    Qt5::Widgets
    Qt5::Svg
    KF5::KCMUtils
    KF5::I18n
    KF5::IconThemes
    KF5::Archive
    KF5::NewStuff
    KF5::KIOWidgets
    KF5::KDELibs4Support
    ${X11_LIBRARIES}
)

if(X11_FOUND)
    target_link_libraries(kcm_icons Qt5::X11Extras)
endif()

install(TARGETS kcm_icons  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES icons.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES icons.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )
