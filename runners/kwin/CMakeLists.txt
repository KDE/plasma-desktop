add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_kwin")

set(krunner_kwin_SRCS
    kwin-runner.cpp
)

add_library(krunner_kwin MODULE ${krunner_kwin_SRCS})
kcoreaddons_desktop_to_json(krunner_kwin plasma-runner-kwin.desktop)
target_link_libraries(krunner_kwin
                        Qt::DBus
                        KF5::KIOCore
                        KF5::I18n
                        KF5::Runner)

install(TARGETS krunner_kwin DESTINATION "${KDE_INSTALL_PLUGINDIR}/kf5/krunner")
