# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_desktoppaths\")

kcmutils_add_qml_kcm(kcm_desktoppaths)

target_sources(kcm_desktoppaths PRIVATE
    globalpaths.cpp
    desktoppathssettings.cpp
)

kcmutils_generate_module_data(kcm_desktoppaths
    MODULE_DATA_HEADER desktoppathsdata.h
    MODULE_DATA_CLASS_NAME DesktopPathsData
    SETTINGS_HEADERS desktoppathssettings.h
    SETTINGS_CLASSES DesktopPathsSettings
)

target_link_libraries(kcm_desktoppaths PRIVATE
    KF6::ConfigCore
    KF6::KCMUtilsQuick
    KF6::I18n
    KF6::KCMUtils
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::KIOFileWidgets
)

add_test(NAME kcm_desktoppaths_smoketest COMMAND kcmshell6 --smoke-test kcm_desktoppaths)
set_tests_properties(kcm_desktoppaths_smoketest PROPERTIES
    ENVIRONMENT_MODIFICATION QT_PLUGIN_PATH=path_list_prepend:${CMAKE_BINARY_DIR}/bin
)
