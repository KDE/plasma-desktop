set(knetattach_SRCS main.cpp knetattach.cpp)


ki18n_wrap_ui(knetattach_SRCS knetattach.ui )

# kde4_add_app_icon(knetattach_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/hi*-app-knetattach.png")

add_executable(knetattach ${knetattach_SRCS})

target_compile_definitions(knetattach PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(knetattach KF5::KIOWidgets KF5::I18n KF5::Codecs)

install(TARGETS knetattach DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( PROGRAMS org.kde.knetattach.desktop  DESTINATION  ${KDE_INSTALL_APPDIR} )

