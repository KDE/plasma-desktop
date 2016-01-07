# - Try to find the SCIM
# Once done this will define
#
#  SCIM_FOUND - system has SCIM
#  SCIM_INCLUDE_DIR - The include directory to use for the SCIM headers
#  SCIM_LIBRARIES - Link these to use SCIM
#  SCIM_DEFINITIONS - Compiler switches required for using SCIM

# Copyright (c) 2009 Wang Hoi, <zeahot.hoi@gmail.org>
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


if (SCIM_LIBRARIES AND SCIM_INCLUDE_DIR)

   # in cache already
   set(SCIM_FOUND TRUE)

else (SCIM_LIBRARIES AND SCIM_INCLUDE_DIR)

   if (NOT WIN32)
      # use pkg-config to get the directories and then use these values
      # in the FIND_PATH() and FIND_LIBRARY() calls
      find_package(PkgConfig)
      pkg_check_modules(PC_SCIM scim)

      set(SCIM_DEFINITIONS ${PC_SCIM_CFLAGS_OTHER})
   endif (NOT WIN32)

   find_path(SCIM_INCLUDE_DIR scim.h
      PATHS
      ${PC_SCIM_INCLUDEDIR}
      ${PC_SCIM_INCLUDE_DIRS}
      )

   find_library(SCIM_LIBRARIES NAMES scim scim-1.0
      PATHS
      ${PC_SCIM_LIBDIR}
      ${PC_SCIM_LIBRARY_DIRS}
      )

   include(FindPackageHandleStandardArgs)
   FIND_PACKAGE_HANDLE_STANDARD_ARGS(SCIM DEFAULT_MSG SCIM_LIBRARIES SCIM_INCLUDE_DIR )

   mark_as_advanced(SCIM_LIBRARIES SCIM_INCLUDE_DIR)

endif (SCIM_LIBRARIES AND SCIM_INCLUDE_DIR)
