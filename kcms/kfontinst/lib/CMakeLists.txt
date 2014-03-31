set(kfontinst_LIB_SRCS Misc.cpp Fc.cpp Family.cpp Style.cpp File.cpp WritingSystems.cpp)
set(kfontinstui_LIB_SRCS FcEngine.cpp )

add_library(kfontinst SHARED ${kfontinst_LIB_SRCS})
target_link_libraries(kfontinst
    Qt5::Xml
    Qt5::DBus
    KF5::I18n
    KF5::KDE4Support
    ${FREETYPE_LIBRARIES}
    ${FONTCONFIG_LIBRARIES}
    ${XCB_XCB_LIBRARY}
    ${XCB_IMAGE_LIBRARY}
)
set_target_properties(kfontinst PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

add_library(kfontinstui SHARED ${kfontinstui_LIB_SRCS})
target_link_libraries(kfontinstui Qt5::X11Extras KF5::KIOCore KF5::KIOWidgets ${FREETYPE_LIBRARIES} ${FONTCONFIG_LIBRARIES} ${X11_X11_LIB} ${X11_Xft_LIB} kfontinst )
set_target_properties(kfontinstui PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kfontinst kfontinstui ${INSTALL_TARGETS_DEFAULT_ARGS} )
