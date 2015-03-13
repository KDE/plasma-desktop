################# KDE PLUGIN for LibAttica #################
add_definitions(-DTRANSLATION_DOMAIN=\"attica_kde\")

set(attica_kde_SRCS
    kdeplatformdependent.cpp
    attica_plugin_debug.cpp
)

add_library(attica_kde MODULE ${attica_kde_SRCS})
target_link_libraries(attica_kde KF5::KIOWidgets Qt5::Widgets KF5::KCMUtils KF5::Attica KF5::Wallet KF5::I18n )
install(TARGETS attica_kde DESTINATION ${PLUGIN_INSTALL_DIR})

