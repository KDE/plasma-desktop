add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_plasma-desktop")

kcoreaddons_add_plugin(krunner_plasma-desktop SOURCES plasma-desktop-runner.cpp INSTALL_NAMESPACE kf6/krunner)
target_link_libraries(krunner_plasma-desktop
                        Qt::DBus
                        KF6::KIOCore
                        KF6::I18n
                        KF6::KIOGui
                        KF6::Runner)
