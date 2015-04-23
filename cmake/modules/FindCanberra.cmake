# - Find libcanberra's libraries and headers.
# This module defines the following variables:
#
#  CANBERRA_FOUND        - true if libcanberra was found
#  CANBERRA_LIBRARIES    - libcanberra libraries to link against
#  CANBERRA_INCLUDE_DIRS - include path for libcanberra
#
# Copyright (c) 2012 Raphael Kubo da Costa <rakuco@FreeBSD.org>
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
