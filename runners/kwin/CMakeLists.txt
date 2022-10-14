add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_kwin")

kcoreaddons_add_plugin(krunner_kwin SOURCES kwin-runner.cpp kwin-runner.h INSTALL_NAMESPACE kf${QT_MAJOR_VERSION}/krunner)
target_link_libraries(krunner_kwin
                        Qt::DBus
                        KF5::KIOCore
                        KF5::I18n
                        KF5::Runner)
