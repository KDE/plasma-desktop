# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmicons\")

add_subdirectory(tests)

########### next target ###############

set(kcm_icons_PART_SRCS iconthemes.cpp icons.cpp main.cpp )

add_library(kcm_icons MODULE ${kcm_icons_PART_SRCS})

target_link_libraries(kcm_icons
    Qt5::Widgets
    Qt5::Svg
    KF5::KCMUtils
    KF5::I18n
    KF5::IconThemes
    KF5::Archive
    KF5::NewStuff
    KF5::KIOWidgets
    KF5::KDELibs4Support
    ${X11_LIBRARIES}
)

if(X11_FOUND)
    target_link_libraries(kcm_icons Qt5::X11Extras)
endif()

file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/config.h CONTENT "#define CMAKE_INSTALL_FULL_LIBEXECDIR \"${CMAKE_INSTALL_FULL_LIBEXECDIR}\"")

add_executable(plasma-changeicons changeicons.cpp)
target_link_libraries(plasma-changeicons PRIVATE Qt5::Core KF5::KIOWidgets KF5::IconThemes)

install(TARGETS kcm_icons  DESTINATION ${PLUGIN_INSTALL_DIR} )

install( FILES icons.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/icons.knsrc ${CMAKE_BINARY_DIR}/icons.knsrc)
install( FILES ${CMAKE_BINARY_DIR}/icons.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )

install(TARGETS plasma-changeicons DESTINATION ${LIBEXEC_INSTALL_DIR} )
