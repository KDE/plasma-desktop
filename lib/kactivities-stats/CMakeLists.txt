# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

# Extra CMake stuff
find_package (ECM 1.8.0 REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set (KACTIVITIES_CURRENT_ROOT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set (KACTIVITIES_INSTALL_EXPERIMENTAL_HEADERS FALSE)

include (KDEInstallDirs)
include (KDECMakeSettings)
include (KDECompilerSettings)
include (GenerateExportHeader)
include (ECMGenerateHeaders)
include (ECMSetupVersion)
include (ECMPackageConfigHelpers)

# libKActivitiesStats

ecm_setup_version (
   ${KF5_VERSION}
   VARIABLE_PREFIX KACTIVITIESEXPERIMENTALSTATS
   VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kactivitiesexperimentalstats_version.h"
   PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5ActivitiesExperimentalStatsConfigVersion.cmake"
   SOVERSION 1
   )

set (CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5ActivitiesExperimentalStats")

add_subdirectory (src)

# Write out the features
feature_summary (WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

