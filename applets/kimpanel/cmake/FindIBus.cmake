# - Try to find IBUS
# Once done this will define
#
#  IBUS_FOUND - system has IBUS
#  IBUS_INCLUDE_DIR - The include directory to use for the fontconfig headers
#  IBUS_LIBRARIES - Link these to use IBUS
#  IBUS_DEFINITIONS - Compiler switches required for using IBUS

# Copyright (c) 2011 Ni Hui, <shuizhuyuanluo@126.com>
# Based on Laurent Montel's FindFontConfig.cmake, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if (IBUS_LIBRARIES AND IBUS_INCLUDE_DIR)

   # in cache already
   set(IBUS_FOUND TRUE)

else (IBUS_LIBRARIES AND IBUS_INCLUDE_DIR)

   if (NOT WIN32)
      # use pkg-config to get the directories and then use these values
      # in the FIND_PATH() and FIND_LIBRARY() calls
      find_package(PkgConfig)
      pkg_check_modules(PC_IBUS "ibus-1.0>=1.4.2")

      set(IBUS_DEFINITIONS ${PC_IBUS_CFLAGS_OTHER})
   endif (NOT WIN32)

   if (PC_IBUS_FOUND)

       find_path(IBUS_INCLUDE_DIR ibus.h
          PREFIX ibus-1.0
          PATHS
          ${PC_IBUS_INCLUDEDIR}
          ${PC_IBUS_INCLUDE_DIRS}
          )

       find_library(IBUS_LIBRARIES NAMES ibus-1.0 ibus
          PATHS
          ${PC_IBUS_LIBDIR}
          ${PC_IBUS_LIBRARY_DIRS}
          )
   endif (PC_IBUS_FOUND)

   include(FindPackageHandleStandardArgs)
   FIND_PACKAGE_HANDLE_STANDARD_ARGS(IBUS DEFAULT_MSG IBUS_LIBRARIES IBUS_INCLUDE_DIR)

   mark_as_advanced(IBUS_LIBRARIES IBUS_INCLUDE_DIR)

endif (IBUS_LIBRARIES AND IBUS_INCLUDE_DIR)
