prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${BIN_INSTALL_DIR}
libdir=${LIB_INSTALL_DIR}
includedir=${INCLUDE_INSTALL_DIR}

Name: libKActivitiesExperimentalStats
Description: libKActivitiesStats is a C++ library for using KDE activities
URL: http://www.kde.org
Requires:
Version: ${KACTIVITIESSTATS_LIB_VERSION_STRING}
Libs: -L${LIB_INSTALL_DIR} -lKActivitiesExperimentalStats
Cflags: -I${INCLUDE_INSTALL_DIR}
