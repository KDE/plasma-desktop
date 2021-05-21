# - Try to find the UDev library
# Once done this will define
#
#  UDEV_FOUND - system has UDev
#  UDEV_INCLUDE_DIR - the libudev include directory
#  UDEV_LIBS - The libudev libraries

# SPDX-FileCopyrightText: 2010 Rafael Fernández López <ereslibre@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

find_path(UDEV_INCLUDE_DIR libudev.h)
find_library(UDEV_LIBS udev)

if(UDEV_INCLUDE_DIR AND UDEV_LIBS)
   include(CheckFunctionExists)
   include(CMakePushCheckState)
   cmake_push_check_state()
   set(CMAKE_REQUIRED_LIBRARIES ${UDEV_LIBS} )

   cmake_pop_check_state()

endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(UDev DEFAULT_MSG UDEV_INCLUDE_DIR UDEV_LIBS)

mark_as_advanced(UDEV_INCLUDE_DIR UDEV_LIBS)
