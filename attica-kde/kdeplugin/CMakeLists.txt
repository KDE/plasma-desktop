add_definitions(-DTRANSLATION_DOMAIN=\"attica_kde_plugin\")

ecm_qt_declare_logging_category(SRCS
    HEADER "attica_plugin_debug.h"
    IDENTIFIER "ATTICA_PLUGIN_LOG"
    CATEGORY_NAME "log_attica_plugin"
)

add_library(attica_kde MODULE kdeplatformdependent.cpp ${SRCS})
target_link_libraries(attica_kde KF5::KIOWidgets Qt5::Widgets KF5::KCMUtils KF5::Attica KF5::Wallet KF5::I18n )

install(TARGETS attica_kde DESTINATION ${KDE_INSTALL_PLUGINDIR})

