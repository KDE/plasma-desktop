add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_plasma-desktop")

kcoreaddons_add_plugin(krunner_plasma-desktop SOURCES plasma-desktop-runner.cpp INSTALL_NAMESPACE kf${QT_MAJOR_VERSION}/krunner)
target_link_libraries(krunner_plasma-desktop
                        Qt::DBus
                        KF5::KIOCore
                        KF5::I18n
                        KF5::KIOGui
                        KF5::Runner)
