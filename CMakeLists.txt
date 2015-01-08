project(kcm-touchpad)
set(PROJECT_VERSION "5.1.95")
cmake_minimum_required(VERSION 2.8.12)

set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
    ${CMAKE_MODULE_PATH}
)
set(CMAKE_INCLUDE_CURRENT_DIR true)
set(TOUCHPAD_KCM_VERSION ${PROJECT_VERSION})

find_package(ECM 0.0.10 REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMOptionalAddSubdirectory)
include(ECMInstallIcons)

find_package(Qt5 REQUIRED
	     X11Extras
	    )

find_package(KDED REQUIRED)

find_package(KF5 REQUIRED
	     Init
	     KCMUtils
	     CoreAddons
	     ConfigWidgets
	     DBusAddons
	     Completion
	     WidgetsAddons
	     I18n
	     Service
	     NotifyConfig
	     XmlGui
	     Notifications
	     GlobalAccel
	     Plasma
	    )


add_subdirectory(src)
add_subdirectory(icon)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/l10n.cmake)
