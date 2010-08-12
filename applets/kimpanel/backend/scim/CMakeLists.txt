project(scim_panel_dbus)
set(QT_USE_QTGUI FALSE)
set(QT_USE_QTDBUS TRUE)
find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
find_package(SCIM REQUIRED)
add_definitions(-DQT_NO_KEYWORDS ${QT_DEFINITIONS})
remove_definitions(-DQT_NO_STL)
include_directories(${QT_INCLUDES} ${SCIM_INCLUDE_DIR})
set(SCIM_ICON_DIR "${PC_SCIM_PREFIX}/share/scim/icons")
if (NOT SCIM_PANEL_INSTALL_DIR)
   set(SCIM_PANEL_INSTALL_DIR "${PC_SCIM_LIBDIR}/scim-1.0/")
endif (NOT SCIM_PANEL_INSTALL_DIR)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-scim.h.cmake
   ${CMAKE_CURRENT_BINARY_DIR}/config-scim.h
   )
set(scim_panel_dbus_SRCS main.cpp)
set(scim_panel_dbus_MOC_SRCS "${CMAKE_CURRENT_BINARY_DIR}/main.moc")
qt4_generate_moc(${scim_panel_dbus_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/main.moc)
kde4_add_executable(scim-panel-dbus ${scim_panel_dbus_SRCS} ${scim_panel_dbus_MOC_SRCS})
target_link_libraries(scim-panel-dbus
   ${QT_LIBRARIES}
   ${SCIM_LIBRARIES}
   )
install(TARGETS scim-panel-dbus
   DESTINATION ${SCIM_PANEL_INSTALL_DIR}
   )
