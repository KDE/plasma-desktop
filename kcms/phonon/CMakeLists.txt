find_package(Phonon4Qt5 4.6.60 NO_MODULE)
include_directories(BEFORE ${PHONON_INCLUDES})
add_definitions(-DPHONON_LIB_SONAME=\"${PHONON_LIB_SONAME}\")
# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm5_phonon\")


set(kcmphonon_SRCS main.cpp devicepreference.cpp backendselection.cpp)
ki18n_wrap_ui(kcmphonon_SRCS devicepreference.ui backendselection.ui)
set(kcmphonon_LIBS
    KF5::ConfigWidgets
    KF5::I18n
    KF5::IconThemes
    KF5::KCMUtils
    KF5::KIOWidgets
    ${PHONON_LIBRARY})

ecm_qt_declare_logging_category(kcmphonon_SRCS HEADER phonon_debug.h IDENTIFIER KCM_PHONON_LOG CATEGORY_NAME org.kde.kcm.phonon)

add_library(kcm_phonon MODULE ${kcmphonon_SRCS})
target_link_libraries(kcm_phonon ${kcmphonon_LIBS} KF5::ConfigWidgets)

install(TARGETS kcm_phonon  DESTINATION ${KDE_INSTALL_PLUGINDIR} )

########### install files ###############
install( FILES kcm_phonon.desktop  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR} )
install(FILES listview-background.png DESTINATION ${KDE_INSTALL_DATADIR}/kcm_phonon)
