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
