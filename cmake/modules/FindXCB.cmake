# Try to find XCB on a Unix system
#
# This will define:
#
#   XCB_FOUND       - True if xcb is available
#   XCB_LIBRARIES   - Link these to use xcb
#   XCB_INCLUDE_DIR - Include directory for xcb
#   XCB_DEFINITIONS - Compiler flags for using xcb
#
# In addition the following more fine grained variables will be defined:
#
#   XCB_XCB_FOUND        XCB_XCB_INCLUDE_DIR        XCB_XCB_LIBRARIES
#   XCB_RECORD_FOUND     XCB_RECORD_INCLUDE_DIR     XCB_RECORD_LIBRARIES
#
# Copyright (c) 2012 Fredrik Höglund <fredrik@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


IF (NOT WIN32)
  IF (XCB_INCLUDE_DIR AND XCB_LIBRARIES)
    # In the cache already
    SET(XCB_FIND_QUIETLY TRUE)
  ENDIF (XCB_INCLUDE_DIR AND XCB_LIBRARIES)

  # Use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  FIND_PACKAGE(PkgConfig)
  PKG_CHECK_MODULES(PKG_XCB QUIET xcb)

  SET(XCB_DEFINITIONS ${PKG_XCB_CFLAGS})

  FIND_PATH(XCB_XCB_INCLUDE_DIR         NAMES xcb/xcb.h             HINTS ${PKG_XCB_INCLUDE_DIRS})
  FIND_PATH(XCB_RECORD_INCLUDE_DIR      NAMES xcb/record.h          HINTS ${PKG_XCB_INCLUDE_DIRS})

  FIND_LIBRARY(XCB_XCB_LIBRARIES         NAMES xcb              HINTS ${PKG_XCB_LIBRARY_DIRS})
  FIND_LIBRARY(XCB_RECORD_LIBRARIES      NAMES xcb-record       HINTS ${PKG_XCB_LIBRARY_DIRS})

  set(XCB_INCLUDE_DIR ${XCB_XCB_INCLUDE_DIR} ${XCB_RECORD_INCLUDE_DIR})

  set(XCB_LIBRARIES ${XCB_XCB_LIBRARIES} ${XCB_RECORD_LIBRARIES})

  list(REMOVE_DUPLICATES XCB_INCLUDE_DIR)

  include(FindPackageHandleStandardArgs)

  FIND_PACKAGE_HANDLE_STANDARD_ARGS(XCB_XCB         DEFAULT_MSG  XCB_XCB_LIBRARIES         XCB_XCB_INCLUDE_DIR)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(XCB_RECORD      DEFAULT_MSG  XCB_RECORD_LIBRARIES      XCB_RECORD_INCLUDE_DIR)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(XCB             DEFAULT_MSG  XCB_LIBRARIES             XCB_INCLUDE_DIR)

  MARK_AS_ADVANCED(
        XCB_INCLUDE_DIR             XCB_LIBRARIES
        XCB_XCB_INCLUDE_DIR         XCB_XCB_LIBRARIES
        XCB_RECORD_INCLUDE_DIR      XCB_RECORD_LIBRARIES
  )

ENDIF (NOT WIN32)

