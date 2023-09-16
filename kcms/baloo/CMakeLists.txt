add_definitions(-DTRANSLATION_DOMAIN=\"kcm_baloofile\")

kcmutils_add_qml_kcm(kcm_baloofile)

target_sources(kcm_baloofile PRIVATE
    kcm.cpp
    fileexcludefilters.cpp
    filteredfoldermodel.cpp
    baloodata.cpp
)

target_link_libraries(kcm_baloofile PRIVATE
    KF6::CoreAddons
    KF6::KCMUtils
    KF6::I18n
    KF6::Solid
    KF6::Baloo
    KF6::KCMUtilsQuick
    KF6::KIOCore
    Qt::DBus
    Qt::Widgets
)
