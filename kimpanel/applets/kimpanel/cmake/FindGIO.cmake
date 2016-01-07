# - Try to find the GIO libraries
# Once done this will define
#
#  GIO_FOUND - system has GIO
#  GIO_INCLUDE_DIR - the GIO include directory
#  GIO_LIBRARIES - GIO library
#
# Copyright (c) 2010 Dario Freddi <drf@kde.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the University nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

if(GIO_INCLUDE_DIR AND GIO_LIBRARIES)
    # Already in cache, be silent
    set(GIO_FIND_QUIETLY TRUE)
endif(GIO_INCLUDE_DIR AND GIO_LIBRARIES)

find_package(PkgConfig)
pkg_check_modules(PC_LibGIO QUIET gio-2.0)

find_path(GIO_MAIN_INCLUDE_DIR
          NAMES glib.h
          HINTS ${PC_LibGIO_INCLUDEDIR}
          PATH_SUFFIXES glib-2.0)

find_library(GIO_LIBRARIES
             NAMES gio-2.0
             HINTS ${PC_LibGLIB2_LIBDIR})


get_filename_component(GIOLibDir "${GIO_LIBRARIES}" PATH)

find_path(GIO_INTERNAL_INCLUDE_DIR glibconfig.h
          PATH_SUFFIXES glib-2.0/include
          PATHS ${_LibGIOIncDir} "${GIOLibDir}" ${CMAKE_SYSTEM_LIBRARY_PATH}
          NO_DEFAULT_PATH)

find_path(GIO_INTERNAL_INCLUDE_DIR glibconfig.h
          PATH_SUFFIXES glib-2.0/include
          PATHS ${_LibGIOIncDir} "${GIOLibDir}" ${CMAKE_SYSTEM_LIBRARY_PATH})

set(GIO_INCLUDE_DIR "${GIO_MAIN_INCLUDE_DIR}")

# not sure if this include dir is optional or required
# for now it is optional
if(GIO_INTERNAL_INCLUDE_DIR)
  set(GIO_INCLUDE_DIR ${GIO_INCLUDE_DIR} "${GIO_INTERNAL_INCLUDE_DIR}")
endif(GIO_INTERNAL_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GIO  DEFAULT_MSG  GIO_LIBRARIES GIO_MAIN_INCLUDE_DIR)

mark_as_advanced(GIO_INCLUDE_DIR GIO_LIBRARIES)
