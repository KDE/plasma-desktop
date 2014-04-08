################# KDE PLUGIN for LibAttica #################

set(attica_kde_SRCS
    kdeplatformdependent.cpp
)

add_library(attica_kde MODULE ${attica_kde_SRCS})
target_link_libraries(attica_kde KF5::KIOWidgets Qt5::Widgets KF5::KCMUtils KF5::KDE4Support)
install(TARGETS attica_kde EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

