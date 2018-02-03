add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_kwin")

set(krunner_kwin_SRCS
    kwin-runner.cpp
)

add_library(krunner_kwin MODULE ${krunner_kwin_SRCS})
target_link_libraries(krunner_kwin
                        Qt5::DBus
                        KF5::KIOCore
                        KF5::I18n
                        KF5::Runner)

install(TARGETS krunner_kwin DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES plasma-runner-kwin.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
