# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (KActivitiesStats)

find_package (Qt5 REQUIRED NO_MODULE COMPONENTS Sql)

set (
   KActivitiesStats_LIB_SRCS

   query.cpp
   terms.cpp
   resultset.cpp
   resultwatcher.cpp
   resultmodel.cpp
   activitiessync_p.cpp
   cleaning.cpp

   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/database/Database.cpp
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/database/schema/ResourcesDatabaseSchema.cpp
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/utils/qsqlquery_iterator.cpp

   version.cpp
   )

qt5_add_dbus_interface (
   KActivitiesStats_LIB_SRCS

   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.ResourcesScoring.xml
   resourcesscoring_interface
   )

qt5_add_dbus_interface (
   KActivitiesStats_LIB_SRCS

   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.ResourcesLinking.xml
   resourceslinking_interface
   )



add_library (
   KF5ActivitiesExperimentalStats SHARED
   ${KActivitiesStats_LIB_SRCS}
   )
add_library(KF5::ActivitiesExperimentalStats ALIAS KF5ActivitiesExperimentalStats)

include_directories (
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src
   ${KDBusAddons_INCLUDE_DIR}
   )

set_target_properties (
   KF5ActivitiesExperimentalStats
   PROPERTIES
   VERSION "0.0.1"
   SOVERSION "1"
   EXPORT_NAME ActivitiesExperimentalStats
   )

target_link_libraries (
   KF5ActivitiesExperimentalStats
   PUBLIC
      Qt5::Core
   PRIVATE
      Qt5::DBus
      Qt5::Sql
      KF5::Activities
      KF5::DBusAddons
   )

target_include_directories (
   KF5ActivitiesExperimentalStats
   INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KActivitiesStats>"
   )

# install
generate_export_header (KF5ActivitiesExperimentalStats BASE_NAME KActivitiesStats)

ecm_generate_headers (
   KActivitiesStats_CamelCase_HEADERS
   HEADER_NAMES
   Query
   Terms
   ResultSet
   ResultWatcher
   ResultModel
   Version
   PREFIX KActivitiesExperimentalStats
   REQUIRED_HEADERS KActivitiesStats_HEADERS
)

if (NOT KACTIVITIES_INSTALL_EXPERIMENTAL_HEADERS)
    set (SKIP_NAMELINK LIBRARY NAMELINK_SKIP)
endif ()

install (
   TARGETS KF5ActivitiesExperimentalStats
   EXPORT KF5ActivitiesExperimentalStatsLibraryTargets
   ${SKIP_NAMELINK}
   ${KF5_INSTALL_TARGETS_DEFAULT_ARGS}
   )

if (KACTIVITIES_INSTALL_EXPERIMENTAL_HEADERS)
   install (
      FILES ${KActivitiesStats_CamelCase_HEADERS}
      DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KActivitiesStats/KActivities/Experimental/Stats
      COMPONENT Devel
      )

   install (
      FILES ${KActivitiesStats_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/kactivitiesstats_export.h
      DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KActivitiesStats/kactivitiesexperimentalstats
      COMPONENT Devel
      )

   if (NOT WIN32)
      configure_file (
         ${CMAKE_CURRENT_SOURCE_DIR}/libKActivitiesExperimentalStats.pc.cmake
         ${CMAKE_CURRENT_BINARY_DIR}/libKActivitiesExperimentalStats.pc
         )
      install (
         FILES ${CMAKE_CURRENT_BINARY_DIR}/libKActivitiesExperimentalStats.pc
         DESTINATION ${KDE_INSTALL_LIBDIR}/pkgconfig
         )
   endif ()

   include (ECMGeneratePriFile)
   ecm_generate_pri_file (
      BASE_NAME KActivitiesExperimentalStats
      LIB_NAME KF5ActivitiesExperimentalStats
      DEPS "KDBusAddons"
      FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KF5_INCLUDE_INSTALL_DIR}/KActivitiesExperimentalStats
      )
   install (
      FILES ${PRI_FILENAME}
      DESTINATION ${ECM_MKSPECS_INSTALL_DIR}
      )
endif ()


