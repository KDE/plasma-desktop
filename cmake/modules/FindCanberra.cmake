# - Find libcanberra's libraries and headers.
# This module defines the following variables:
#
#  CANBERRA_FOUND        - true if libcanberra was found
#  CANBERRA_LIBRARIES    - libcanberra libraries to link against
#  CANBERRA_INCLUDE_DIRS - include path for libcanberra
#
# Copyright (c) 2012 Raphael Kubo da Costa <rakuco@FreeBSD.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_package(PkgConfig)
pkg_check_modules(PC_CANBERRA libcanberra)

find_library(CANBERRA_LIBRARIES
    NAMES canberra
    HINTS ${PC_CANBERRA_LIBRARY_DIRS} ${PC_CANBERRA_LIBDIR}
)

find_path(CANBERRA_INCLUDE_DIRS
    NAMES canberra.h
    HINTS ${PC_CANBERRA_INCLUDE_DIRS} ${PC_CANBERRA_INCLUDEDIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Canberra REQUIRED_VARS CANBERRA_LIBRARIES CANBERRA_INCLUDE_DIRS)

mark_as_advanced(CANBERRA_LIBRARIES CANBERRA_INCLUDE_DIRS)
