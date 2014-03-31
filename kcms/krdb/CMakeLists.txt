
add_subdirectory( themes ) 
add_subdirectory( ad ) 
add_subdirectory( pics ) 

install(FILES krdb_libpathwipe.upd DESTINATION ${DATA_INSTALL_DIR}/kconf_update)

add_executable(krdb_clearlibrarypath krdb_clearlibrarypath.cpp)
target_link_libraries(krdb_clearlibrarypath Qt5::Core KF5::KDE4Support)
install(TARGETS krdb_clearlibrarypath DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin)
