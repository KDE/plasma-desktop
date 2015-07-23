add_definitions(-DTRANSLATION_DOMAIN=\"useraccount\")

add_subdirectory(pics)

set(kcm_useraccount_PART_SRCS
    main.cpp
    faceiconbutton.cpp
    faceiconpopup.cpp
    pwdedit.cpp
    chgpwddlg.cpp
    adduserdlg.cpp
    removeuserdlg.cpp)

kconfig_add_kcfg_files(kcm_useraccount_PART_SRCS settings.kcfgc)

add_library(kcm_useraccount MODULE ${kcm_useraccount_PART_SRCS})

include_directories(${POLKITQT-1_INCLUDE_DIR})

target_include_directories(kcm_useraccount PUBLIC "$<BUILD_INTERFACE:${Kdepasswd_BINARY_DIR}>")

target_link_libraries(kcm_useraccount
    KF5::KDELibs4Support
    KF5::KIOWidgets
    KF5::Wallet
    QtAccountsService::QtAccountsService
    ${POLKITQT-1_LIBRARIES}
    -lcrypt
)

install(TARGETS kcm_useraccount DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_useraccount.desktop DESTINATION  ${SERVICES_INSTALL_DIR})
