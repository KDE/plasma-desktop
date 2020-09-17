# - Try to find IBUS
# Once done this will define
#
#  IBUS_FOUND - system has IBUS
#  IBUS_INCLUDE_DIR - The include directory to use for the IBUS headers
#  IBUS_LIBRARIES - Link these to use IBUS
#  IBUS_DEFINITIONS - Compiler switches required for using IBUS

# Copyright (c) 2011 Ni Hui, <shuizhuyuanluo@126.com>
# Based on Laurent Montel's FindFontConfig.cmake, <montel@kde.org>
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


if (IBUS_LIBRARIES AND IBUS_INCLUDE_DIR AND IBUS_VERSION)

   # in cache already
   set(IBUS_FOUND TRUE)

else ()

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

   set(IBUS_VERSION ${PC_IBUS_VERSION})
   set(IBUS_FOUND ${PC_IBUS_FOUND})

   include(FindPackageHandleStandardArgs)
   FIND_PACKAGE_HANDLE_STANDARD_ARGS(IBUS
                                     FOUND_VAR IBUS_FOUND
                                     REQUIRED_VARS IBUS_LIBRARIES IBUS_INCLUDE_DIR
                                     VERSION_VAR IBUS_VERSION)

   mark_as_advanced(IBUS_LIBRARIES IBUS_INCLUDE_DIR IBUS_VERSION)

endif ()
