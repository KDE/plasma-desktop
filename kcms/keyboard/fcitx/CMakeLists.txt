set(kcm_keyboard_fcitx_SRCS
    fcitxqtinputmethoditem.cpp
)

qt5_add_dbus_interface(kcm_keyboard_fcitx_SRCS
    org.fcitx.Fcitx.InputMethod.xml
    fcitx_interface)

set_source_files_properties(
    org.fcitx.Fcitx.InputMethod.xml
    PROPERTIES
        CLASSNAME FcitxQtInputMethodProxy
        INCLUDE fcitxqtinputmethoditem.h
        NO_NAMESPACE ON
)

ecm_qt_declare_logging_category(kcm_keyboard_fcitx_SRCS
    HEADER debug.h
    IDENTIFIER KCM_KEYBOARD
    CATEGORY_NAME org.kde.kcm_keyboard
    DEFAULT_SEVERITY Warning)

add_library(kcm_keyboard_fcitx STATIC ${kcm_keyboarb_fcitx_SRCS})

target_link_libraries(kcm_keyboard_fcitx
    Qt5::Core
    Qt5::DBus
)

target_include_directories(kcm_keyboard_fcitx PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR})
