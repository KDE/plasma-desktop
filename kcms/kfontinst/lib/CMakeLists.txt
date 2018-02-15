set(kfontinst_LIB_SRCS Misc.cpp Fc.cpp Family.cpp Style.cpp File.cpp WritingSystems.cpp)
set(kfontinstui_LIB_SRCS FcEngine.cpp )

configure_file(config-paths.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-paths.h)

add_library(kfontinst SHARED ${kfontinst_LIB_SRCS})
target_link_libraries(kfontinst
    Qt5::Xml
    Qt5::DBus
    Qt5::Gui
    KF5::I18n
    KF5::KIOCore
    ${FREETYPE_LIBRARIES}
    ${FONTCONFIG_LIBRARIES}
    XCB::XCB
    XCB::IMAGE
)
set_target_properties(kfontinst PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR} )

add_library(kfontinstui SHARED ${kfontinstui_LIB_SRCS})
target_link_libraries(kfontinstui Qt5::X11Extras KF5::KIOCore KF5::KIOWidgets ${FREETYPE_LIBRARIES} ${FONTCONFIG_LIBRARIES} ${X11_X11_LIB} ${X11_Xft_LIB} kfontinst )
set_target_properties(kfontinstui PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR} )
install(TARGETS kfontinst kfontinstui ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
