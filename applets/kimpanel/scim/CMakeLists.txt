project(scim_panel_dbus)
find_package(Qt4 REQUIRED)
add_definitions(${QT_DEFINITIONS} -DQT_NO_KEYWORDS -pthread)
include_directories(${QT_INCLUDES} /usr/include/scim-1.0/)
link_directories(/usr/lib/scim-1.0/)
include(${QT_USE_FILE})
set(scim_panel_dbus_SRCS
   main.cpp
   )
set(scim_panel_dbus_MOC_SRCS "${CMAKE_CURRENT_BINARY_DIR}/main.moc")
qt4_generate_moc(${scim_panel_dbus_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/main.moc)
add_executable(scim-panel-dbus ${scim_panel_dbus_SRCS} ${scim_panel_dbus_MOC_SRCS})
target_link_libraries(scim-panel-dbus 
   ${QT_QTCORE_LIBRARY} 
   ${QT_QTDBUS_LIBRARY}
   scim-1.0
   )

