add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_kwin")

kcoreaddons_add_plugin(krunner_kwin SOURCES kwin-runner.cpp INSTALL_NAMESPACE "kf5/krunner")
kcoreaddons_desktop_to_json(krunner_kwin plasma-runner-kwin.desktop)
target_link_libraries(krunner_kwin
                        Qt::DBus
                        KF5::KIOCore
                        KF5::I18n
                        KF5::Runner)
