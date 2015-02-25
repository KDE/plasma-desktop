# - Find synaptics's libraries and headers.
# This module defines the following variables:
#
#  SYNAPTICS_FOUND        - true if synaptics was found
#  SYNAPTICS_INCLUDE_DIRS - include path for synaptics
#  There are no libraries, just a header file
#
# Copyright (c) 2015 David Edmundson <davidedmundson@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_package(PkgConfig)
pkg_check_modules(PC_SYNAPTICS xorg-synaptics)

find_path(Synaptics_INCLUDE_DIRS
    NAMES synaptics-properties.h
    HINTS ${PC_SYNAPTICS_INCLUDE_DIRS} ${PC_SYNAPTICS_INCLUDEDIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Synaptics REQUIRED_VARS Synaptics_INCLUDE_DIRS)

mark_as_advanced(Synaptics_INCLUDE_DIRS)
