add_definitions(-DTRANSLATION_DOMAIN=\"kcmtranslations\")

set(kcm_translations_PART_SRCS kcmtranslations.cpp)

ki18n_wrap_ui(kcm_translations_PART_SRCS kcmtranslationswidget.ui)

add_library(kcm_translations ${kcm_translations_PART_SRCS})

target_link_libraries(kcm_translations
    Qt5::Widgets
    KF5::WidgetsAddons
    KF5::KCMUtils
    KF5::I18n
)

########### install files ###############

install(TARGETS kcm_translations  DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES translations.desktop  DESTINATION  ${SERVICES_INSTALL_DIR})
