set(kcm_spellchecking_SRCS
   spellchecking.cpp
   )

add_library(kcmspellchecking MODULE ${kcm_spellchecking_SRCS})

target_link_libraries(kcmspellchecking
                      KF5::SonnetUi
                      KF5::KDELibs4Support
                     )

install(TARGETS kcmspellchecking   DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES spellchecking.desktop   DESTINATION  ${SERVICES_INSTALL_DIR} )
