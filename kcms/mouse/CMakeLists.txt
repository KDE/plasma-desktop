# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmmouse\")

add_subdirectory(misc)

kcmutils_add_qml_kcm(kcm_mouse
    SOURCES
        kcm.cpp
        kcm.h
        inputbackend.cpp
        inputbackend.h
        inputdevice.cpp
        inputdevice.h
        mousemoduledata.cpp
        mousemoduledata.h
)

ecm_qt_declare_logging_category(kcm_mouse
    HEADER logging.h
    IDENTIFIER KCM_MOUSE
    CATEGORY_NAME kcm_mouse
    DESCRIPTION "KCM for mouse input"
    EXPORT KCM_MOUSE
)

ecm_qt_install_logging_categories(
    EXPORT KCM_MOUSE
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

target_link_libraries(kcm_mouse PRIVATE
    KF6::I18n
    KF6::KCMUtilsQuick
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::WindowSystem
    Qt::QuickWidgets
)

add_subdirectory(backends)
