add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_plasma-desktop")

set(krunner_plasma-desktop_SRCS
    plasma-desktop-runner.cpp
)

add_library(krunner_plasma-desktop MODULE ${krunner_plasma-desktop_SRCS})
kcoreaddons_desktop_to_json(krunner_plasma-desktop plasma-runner-plasma-desktop.desktop)
target_link_libraries(krunner_plasma-desktop
                        Qt::DBus
                        KF5::KIOCore
                        KF5::I18n
                        KF5::Runner)

install(TARGETS krunner_plasma-desktop DESTINATION "${KDE_INSTALL_PLUGINDIR}/kf5/krunner")
