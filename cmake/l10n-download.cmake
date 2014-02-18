set(poFile ${CMAKE_CURRENT_BINARY_DIR}/${CATALOG}_${LANG_CODE}.po)
set(srcPoFile ${CMAKE_CURRENT_SOURCE_DIR}/${CATALOG}_${LANG_CODE}.po)

if(EXISTS ${srcPoFile})
    copy_if_different(${srcPoFile} ${poFile})
else()
    file(DOWNLOAD
         http://websvn.kde.org/*checkout*/trunk/l10n-kde4/${LANG_CODE}/messages/playground-utils/${CATALOG}.po
         ${poFile}
         SHOW_PROGRESS
    )
endif()
