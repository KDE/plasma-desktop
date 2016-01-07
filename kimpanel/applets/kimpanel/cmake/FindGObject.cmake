# - Try to find GObject
# Once done this will define
#
#  GOBJECT_FOUND - system has GObject
#  GOBJECT_INCLUDE_DIR - the GObject include directory
#  GOBJECT_LIBRARIES - the libraries needed to use GObject
#  GOBJECT_DEFINITIONS - Compiler switches required for using GObject

# Copyright (c) 2011, Raphael Kubo da Costa <kubito@gmail.com>
# Copyright (c) 2006, Tim Beaulen <tbscope@gmail.com>
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

FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(PC_GOBJECT gobject-2.0)
SET(GOBJECT_DEFINITIONS ${PC_GOBJECT_CFLAGS_OTHER})

FIND_PATH(GOBJECT_INCLUDE_DIR gobject.h
   HINTS
   ${PC_GOBJECT_INCLUDEDIR}
   ${PC_GOBJECT_INCLUDE_DIRS}
   PATH_SUFFIXES glib-2.0/gobject/
   )

FIND_LIBRARY(_GObjectLibs NAMES gobject-2.0
   HINTS
   ${PC_GOBJECT_LIBDIR}
   ${PC_GOBJECT_LIBRARY_DIRS}
   )
FIND_LIBRARY(_GModuleLibs NAMES gmodule-2.0
   HINTS
   ${PC_GOBJECT_LIBDIR}
   ${PC_GOBJECT_LIBRARY_DIRS}
   )
FIND_LIBRARY(_GThreadLibs NAMES gthread-2.0
   HINTS
   ${PC_GOBJECT_LIBDIR}
   ${PC_GOBJECT_LIBRARY_DIRS}
   )
FIND_LIBRARY(_GLibs NAMES glib-2.0
   HINTS
   ${PC_GOBJECT_LIBDIR}
   ${PC_GOBJECT_LIBRARY_DIRS}
   )

SET( GOBJECT_LIBRARIES ${_GObjectLibs} ${_GModuleLibs} ${_GThreadLibs} ${_GLibs} )

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GOBJECT DEFAULT_MSG GOBJECT_LIBRARIES GOBJECT_INCLUDE_DIR)

MARK_AS_ADVANCED(GOBJECT_INCLUDE_DIR _GObjectLibs _GModuleLibs _GThreadLibs _GLibs)
