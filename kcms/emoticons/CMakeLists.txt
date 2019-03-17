# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm5_emoticons\")

set(kcmemoticons_SRCS emoticonslist.cpp)
ki18n_wrap_ui(kcmemoticons_SRCS emoticonslist.ui)

add_library(kcm_emoticons MODULE ${kcmemoticons_SRCS})
target_link_libraries(kcm_emoticons
                      KF5::NewStuff
                      KF5::KDELibs4Support
                      KF5::Emoticons)

install(TARGETS kcm_emoticons  DESTINATION ${KDE_INSTALL_PLUGINDIR} )

########### install files ###############
install( FILES emoticons.desktop  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR} )
install( FILES emoticons.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR} )
