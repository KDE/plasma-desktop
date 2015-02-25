#
# Copyright (C) 2013 Alexander Mezin <mezin.alexander@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

set(TRANSLATIONS "" CACHE STRING "")

if(NOT "${TRANSLATIONS}" STREQUAL "")
    find_program(GETTEXT_MSGFMT_COMMAND msgfmt REQUIRED)
    add_custom_target(translations ALL)

    set(translationList ${TRANSLATIONS})
    separate_arguments(translationList)
    foreach(langCode ${translationList})
        message("Building translation ${langCode}")

        foreach(catalog kcm_touchpad plasma_applet_touchpad)
            set(poFile ${CMAKE_CURRENT_BINARY_DIR}/${catalog}_${langCode}.po)
            add_custom_command(OUTPUT ${poFile}
                               COMMAND ${CMAKE_COMMAND} ARGS
                               -DLANG_CODE=${langCode}
                               -DCATALOG=${catalog}
                               -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/l10n-download.cmake
            )
            set(gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${catalog}_${langCode}.gmo)
            add_custom_command(OUTPUT ${gmoFile}
                               COMMAND ${GETTEXT_MSGFMT_COMMAND}
                               ARGS --check -o ${gmoFile} ${poFile}
                               MAIN_DEPENDENCY ${poFile}
            )
            install(FILES ${gmoFile}
                    DESTINATION ${LOCALE_INSTALL_DIR}/${langCode}/LC_MESSAGES/
                    RENAME ${catalog}.mo
            )
            list(APPEND gmoFiles ${gmoFile})
        endforeach()

        add_custom_target(translation_${langCode} SOURCES ${gmoFiles})
        add_dependencies(translations translation_${langCode})
    endforeach()
endif()
