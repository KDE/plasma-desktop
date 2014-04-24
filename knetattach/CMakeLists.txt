set(knetattach_SRCS main.cpp knetattach.cpp)


qt5_wrap_ui(knetattach_SRCS knetattach.ui )

# kde4_add_app_icon(knetattach_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/hi*-app-knetattach.png")

add_executable(knetattach ${knetattach_SRCS})
ecm_mark_nongui_executable(knetattach)

target_link_libraries(knetattach KF5::KIOWidgets KF5::KDELibs4Support)

install(TARGETS knetattach DESTINATION ${LIBEXEC_INSTALL_DIR})


########### install files ###############

install( PROGRAMS knetattach.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )

ecm_install_icons( ${ICON_INSTALL_DIR}  )

