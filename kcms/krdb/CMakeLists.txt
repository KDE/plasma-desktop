# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"krdb\")

add_subdirectory( ad ) 
add_subdirectory( pics ) 

install(FILES krdb_libpathwipe.upd DESTINATION ${KDE_INSTALL_DATADIR}/kconf_update)

add_executable(krdb_clearlibrarypath krdb_clearlibrarypath.cpp)
target_link_libraries(krdb_clearlibrarypath Qt5::Core KF5::KDELibs4Support)
install(TARGETS krdb_clearlibrarypath DESTINATION ${KDE_INSTALL_LIBDIR}/kconf_update_bin)
