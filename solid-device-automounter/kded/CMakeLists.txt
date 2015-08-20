set(kded_device_automounter_SRCS DeviceAutomounter.cpp)

kconfig_add_kcfg_files(kded_device_automounter_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../lib/AutomounterSettingsBase.kcfgc)

add_library(kded_device_automounter MODULE ${device_automounter_lib_SRCS} ${kded_device_automounter_SRCS})

target_link_libraries(kded_device_automounter KF5::CoreAddons KF5::Service KF5::ConfigCore KF5::ConfigGui KF5::DBusAddons KF5::Solid)

install(TARGETS kded_device_automounter DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES device_automounter.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)
