set(krunner_plasma-desktop_SRCS
    plasma-desktop-runner.cpp
)

add_library(krunner_plasma-desktop MODULE ${krunner_plasma-desktop_SRCS})
target_link_libraries(krunner_plasma-desktop KIOCore KF5::Plasma)

install(TARGETS krunner_plasma-desktop DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-plasma-desktop.desktop DESTINATION ${SERVICES_INSTALL_DIR})

