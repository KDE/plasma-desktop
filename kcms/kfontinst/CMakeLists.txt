if (KFI_BUILD_STANDALONE)
    cmake_minimum_required(VERSION 2.6.4)
    find_package(KDE4 "4.6.0" REQUIRED)
    find_package(Qt4 REQUIRED)
    find_package(Freetype REQUIRED)
    find_package(Fontconfig REQUIRED)
    find_package(Strigi REQUIRED)
    find_package(X11 REQUIRED)
    macro_log_feature(X11_Xft_FOUND "libxft" "X FreeType interface library" "http://www.x.org" FALSE "" "Font installer and font previews")
    include(KDE4Defaults)
endif ()

# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kfontinst\")

if (X11_Xft_FOUND)
    check_include_files(locale.h HAVE_LOCALE_H)
    configure_file(config-fontinst.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-fontinst.h)
    include_directories(
        ${FREETYPE_INCLUDE_DIRS}
        ${FONTCONFIG_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
        ${CMAKE_CURRENT_BINARY_DIR}/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/dbus
        ${CMAKE_CURRENT_BINARY_DIR}/dbus
        ${CMAKE_CURRENT_SOURCE_DIR}/viewpart
        ${CMAKE_CURRENT_SOURCE_DIR}/kcmfontinst
        ${CMAKE_CURRENT_BINARY_DIR}/kcmfontinst)
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

    set(libkfontinstdbusiface_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/dbus/FontinstIface.cpp)
    set(libkfontinstview_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/viewpart/FontPreview.cpp
                              ${CMAKE_CURRENT_SOURCE_DIR}/viewpart/PreviewSelectAction.cpp
                              ${CMAKE_CURRENT_SOURCE_DIR}/viewpart/CharTip.cpp )
    set(libkfontinstactionlabel_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/kcmfontinst/ActionLabel.cpp)
    set(libkfontinstjobrunner_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/kcmfontinst/JobRunner.cpp
                                   ${libkfontinstactionlabel_SRCS}
                                   ${CMAKE_CURRENT_SOURCE_DIR}/kcmfontinst/FontsPackage.cpp)

    add_subdirectory( lib ) 
    add_subdirectory( dbus )
    add_subdirectory( kcmfontinst ) 
    add_subdirectory( apps )
    add_subdirectory( kio ) 
    add_subdirectory( thumbnail )
    add_subdirectory( viewpart )

    ecm_install_icons( ICONS hisc-apps-preferences-desktop-font-installer.svgz DESTINATION ${ICON_INSTALL_DIR} )

endif ()
