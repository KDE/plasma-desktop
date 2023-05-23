add_definitions(-DTRANSLATION_DOMAIN=\"attica_kde_plugin\")

add_library(attica_kde MODULE kdeplatformdependent.cpp)
ecm_qt_declare_logging_category(attica_kde
    HEADER "attica_plugin_debug.h"
    IDENTIFIER "ATTICA_PLUGIN_LOG"
    CATEGORY_NAME "log_attica_plugin"
)
target_link_libraries(attica_kde
    Qt::Widgets
    KF6::KIOWidgets
    KF6::KCMUtils
    KF6::I18n
    KF6::Service
    KF6::AtticaIntegrationPrivate
    KAccounts
)

install(TARGETS attica_kde DESTINATION ${KDE_INSTALL_PLUGINDIR})
kaccounts_add_provider(opendesktop.provider.in)
kaccounts_add_service(opendesktop-rating.service.in)
