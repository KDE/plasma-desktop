kde_enable_exceptions()

add_definitions(-DTRANSLATION_DOMAIN=\"marginSeparator\")

kcoreaddons_add_plugin(org.kde.plasma.marginsseparator SOURCES marginSeparator.cpp INSTALL_NAMESPACE "plasma/applets")

target_link_libraries(org.kde.plasma.marginsseparator Qt::Gui Qt::Core Qt::Qml Qt::Quick KF5::Plasma KF5::PlasmaQuick KF5::I18n)
