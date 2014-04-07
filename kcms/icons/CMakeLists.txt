
add_subdirectory(tests)

########### next target ###############

set(kcm_icons_PART_SRCS iconthemes.cpp icons.cpp main.cpp )

add_library(kcm_icons MODULE ${kcm_icons_PART_SRCS})

target_link_libraries(kcm_icons
    Qt5::Widgets
    Qt5::Svg
    KF5::KCMUtils
    KF5::I18n
    KF5::UnitConversion
    KF5::Archive
    ${X11_LIBRARIES}
    KF5::KDE4Support
    KF5::NewStuff
)

install(TARGETS kcm_icons  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES icons.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES icons.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )
