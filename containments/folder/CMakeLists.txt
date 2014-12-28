add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.folder\")

plasma_install_package(package org.kde.plasma.folder)

include_directories(plugin/plasmaquick)

set(folderplugin_SRCS
    plugin/directorypicker.cpp
    plugin/foldermodel.cpp
    plugin/folderplugin.cpp
    plugin/itemgrabber.cpp
    plugin/itemviewadapter.cpp
    plugin/labelgenerator.cpp
    plugin/menuhelper.cpp
    plugin/mimetypesmodel.cpp
    plugin/placesmodel.cpp
    plugin/positioner.cpp
    plugin/previewpluginsmodel.cpp
    plugin/rubberband.cpp
    plugin/subdialog.cpp
    plugin/systemsettings.cpp
    plugin/viewpropertiesmenu.cpp
    plugin/wheelinterceptor.cpp
    plugin/internallibkonq/konq_copytomenu.cpp
    plugin/internallibkonq/konq_dndpopupmenuplugin.cpp
    plugin/internallibkonq/konq_popupmenu.cpp
    plugin/internallibkonq/konq_popupmenuplugin.cpp
)

install(FILES plugin/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/folder)

add_library(folderplugin SHARED ${folderplugin_SRCS})

target_link_libraries(folderplugin
                      Qt5::Core
                      Qt5::Qml
                      Qt5::Quick
                      Qt5::Widgets
                      KF5::KIOCore
                      KF5::KIOWidgets
                      KF5::KIOFileWidgets
                      KF5::KDELibs4Support
                      KF5::PlasmaQuick)

set(FOLDER_BUILD_OPENGL FALSE)
set(FOLDER_BUILD_OPENGLES FALSE)
if(OPENGL_FOUND AND (${Qt5Gui_OPENGL_IMPLEMENTATION} STREQUAL "GL"))
    set(FOLDER_BUILD_OPENGL TRUE)
endif()
if(OPENGLES_FOUND AND (${Qt5Gui_OPENGL_IMPLEMENTATION} STREQUAL "GLESv2"))
    set(FOLDER_BUILD_OPENGLES TRUE)
endif()

if(FOLDER_BUILD_OPENGL)
  target_link_libraries(folderplugin ${OPENGL_gl_LIBRARY})
  # -ldl used by OpenGL code
  find_library(DL_LIBRARY dl)
  if (DL_LIBRARY)
    target_link_libraries(folderplugin ${DL_LIBRARY})
  endif()
elseif(FOLDER_BUILD_OPENGLES)
  target_link_libraries(folderplugin ${OPENGLES_LIBRARIES})
endif()

install(TARGETS folderplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/folder)
