add_definitions(-DTRANSLATION_DOMAIN=\"attica_kde_plugin\")

ecm_qt_declare_logging_category(SRCS
    HEADER "attica_plugin_debug.h"
    IDENTIFIER "ATTICA_PLUGIN_LOG"
    CATEGORY_NAME "log_attica_plugin"
)

add_library(attica_kde MODULE kdeplatformdependent.cpp ${SRCS})
target_include_directories(attica_kde PRIVATE "${ACCOUNTSQT_INCLUDE_DIRS}")
target_link_libraries(attica_kde
    PUBLIC
        Qt::Widgets
        Qt::Xml
        KF6::KIOWidgets
        KF6::KCMUtils
        KF6::Attica
        KF6::I18n
        KF6::Service
        KAccounts6
        ${ACCOUNTSQT_LIBRARIES}
)

install(TARGETS attica_kde                 DESTINATION ${KDE_INSTALL_PLUGINDIR})
kaccounts_add_provider(opendesktop.provider.in)
kaccounts_add_service(opendesktop-rating.service.in)
