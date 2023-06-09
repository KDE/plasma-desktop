set(kded_device_automounter_SRCS DeviceAutomounter.cpp)

kconfig_add_kcfg_files(kded_device_automounter_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/DeviceSettings.kcfgc
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/AutomounterSettingsBase.kcfgc
)

kcoreaddons_add_plugin(kded_device_automounter SOURCES ${device_automounter_lib_SRCS} ${kded_device_automounter_SRCS} INSTALL_NAMESPACE kf6/kded)
set_target_properties(kded_device_automounter PROPERTIES OUTPUT_NAME device_automounter)
target_link_libraries(kded_device_automounter Qt::DBus KF6::CoreAddons KF6::Service KF6::ConfigCore KF6::ConfigGui KF6::DBusAddons KF6::Solid)
