add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_kwin")

kcoreaddons_add_plugin(krunner_kwin SOURCES kwin-runner.cpp kwin-runner.h INSTALL_NAMESPACE kf6/krunner)
target_link_libraries(krunner_kwin
                        Qt::DBus
                        KF6::KIOCore
                        KF6::I18n
                        KF6::Runner)
