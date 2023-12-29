add_definitions(-DTRANSLATION_DOMAIN=\"kcm_device_automounter\")

set(kcm_device_automounter_SRCS DeviceAutomounterKCM.cpp DeviceModel.cpp DeviceAutomounterKCM.h DeviceModel.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

ki18n_wrap_ui(kcm_device_automounter_SRCS DeviceAutomounterKCM.ui)

kcmutils_generate_module_data(
    kcm_device_automounter_SRCS
    MODULE_DATA_HEADER DeviceAutomounterData.h
    MODULE_DATA_CLASS_NAME DeviceAutomounterData
    SETTINGS_HEADERS AutomounterSettings.h
    SETTINGS_CLASSES AutomounterSettings
)

kconfig_add_kcfg_files(kcm_device_automounter_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/DeviceSettings.kcfgc
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/AutomounterSettingsBase.kcfgc
    ${CMAKE_CURRENT_SOURCE_DIR}/LayoutSettings.kcfgc
)


add_library(kcm_device_automounter MODULE ${kcm_device_automounter_SRCS} ${device_automounter_lib_SRCS})

target_link_libraries(kcm_device_automounter
                      Qt::DBus
                      KF6::CoreAddons
                      KF6::KCMUtils
                      KF6::ConfigWidgets
                      KF6::Service
                      KF6::I18n
                      KF6::Solid)

install(TARGETS kcm_device_automounter DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcms/systemsettings_qwidgets)

