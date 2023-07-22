file(GLOB EMOJI_DATA_FILES "*.dict")
install(FILES ${EMOJI_DATA_FILES} DESTINATION ${KDE_INSTALL_DATADIR}/plasma/emoji)

if(BUILD_TESTING AND CMAKE_SYSTEM_NAME MATCHES "Linux")
add_test(
    NAME emojidataupdatecheck
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/emojidataupdatecheck.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
set_tests_properties(emojidataupdatecheck PROPERTIES TIMEOUT 60)
endif()
