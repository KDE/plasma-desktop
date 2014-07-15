add_definitions(-DTRANSLATION_DOMAIN=\"useraccount\")

add_subdirectory(pics)

set(kcm_useraccount_PART_SRCS
    main.cpp
    chfacedlg.cpp)

ki18n_wrap_ui(kcm_useraccount_PART_SRCS
    main_widget.ui
    faceDlg.ui)

kconfig_add_kcfg_files(kcm_useraccount_PART_SRCS settings.kcfgc pass.kcfgc)

add_library(kcm_useraccount MODULE ${kcm_useraccount_PART_SRCS})

target_include_directories(kcm_useraccount PUBLIC "$<BUILD_INTERFACE:${Kdepasswd_BINARY_DIR}>")

target_link_libraries(kcm_useraccount
    KF5::KDELibs4Support
    KF5::KIOWidgets
)

install(TARGETS kcm_useraccount DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_useraccount.desktop DESTINATION  ${SERVICES_INSTALL_DIR})
install(FILES kcm_useraccount.kcfg kcm_useraccount_pass.kcfg DESTINATION ${KCFG_INSTALL_DIR})
