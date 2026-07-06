set(kcm_spellchecking_SRCS
   spellcheckingskeleton.cpp
   spellchecking.cpp
    spellcheckingdata.cpp
   )

add_library(kcm_spellchecking MODULE ${kcm_spellchecking_SRCS})
kcmutils_generate_desktop_file(kcm_spellchecking)

target_link_libraries(kcm_spellchecking
                      KF6::KCMUtils
                      KF6::SonnetCore
                      KF6::SonnetUi
                     )

install(TARGETS kcm_spellchecking   DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcms/systemsettings_qwidgets )
