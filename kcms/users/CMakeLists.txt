add_definitions(-DTRANSLATION_DOMAIN=\"kcm_users\")

add_subdirectory(src)

install( FILES kcm_users.desktop  DESTINATION  ${SERVICES_INSTALL_DIR})
kpackage_install_package(package kcm_users kcms)
