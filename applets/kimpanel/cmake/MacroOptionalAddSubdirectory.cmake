# - MACRO_OPTIONAL_ADD_SUBDIRECTORY() combines ADD_SUBDIRECTORY() with an OPTION()
# MACRO_OPTIONAL_ADD_SUBDIRECTORY( <dir> )
# If you use MACRO_OPTIONAL_ADD_SUBDIRECTORY() instead of ADD_SUBDIRECTORY(),
# this will have two effects
# 1 - CMake will not complain if the directory doesn't exist
#     This makes sense if you want to distribute just one of the subdirs
#     in a source package, e.g. just one of the subdirs in kdeextragear.
# 2 - If the directory exists, it will offer an option to skip the 
#     subdirectory.
#     This is useful if you want to compile only a subset of all
#     directories.

# Copyright (c) 2007, Alexander Neundorf, <neundorf@kde.org>
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


MACRO (MACRO_OPTIONAL_ADD_SUBDIRECTORY _dir )
   GET_FILENAME_COMPONENT(_fullPath ${_dir} ABSOLUTE)
   IF(EXISTS ${_fullPath})
      OPTION(BUILD_${_dir} "Build directory ${_dir}" TRUE)
      IF(BUILD_${_dir})
         ADD_SUBDIRECTORY(${_dir})
      ENDIF(BUILD_${_dir})
   ENDIF(EXISTS ${_fullPath})
ENDMACRO (MACRO_OPTIONAL_ADD_SUBDIRECTORY)
