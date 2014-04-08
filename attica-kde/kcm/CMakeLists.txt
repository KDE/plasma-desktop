set(kcm_attica_PART_SRCS
    providerconfigwidget.cpp
    atticamodule.cpp
)

qt5_wrap_ui(kcm_attica_PART_SRCS providermanagement.ui providerconfigwidget.ui)

add_library(kcm_attica MODULE ${kcm_attica_PART_SRCS})

target_link_libraries(kcm_attica KF5::Attica KF5::KDE4Support KF5::KIOWidgets KF5::I18n)

install(TARGETS kcm_attica DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_attica.desktop DESTINATION ${SERVICES_INSTALL_DIR})

