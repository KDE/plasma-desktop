set(kcm_spellchecking_SRCS
   spellcheckingskeleton.cpp
   spellchecking.cpp
    spellcheckingdata.cpp
   )

add_library(kcmspellchecking MODULE ${kcm_spellchecking_SRCS})
kcmutils_generate_desktop_file(kcmspellchecking)

target_link_libraries(kcmspellchecking
                      KF6::KCMUtils
                      KF6::SonnetCore
                      KF6::SonnetUi
                     )

install(TARGETS kcmspellchecking   DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcms/systemsettings_qwidgets )
