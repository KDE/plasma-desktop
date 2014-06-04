find_package(Phonon4Qt5 4.6.60 NO_MODULE)
include_directories(BEFORE ${PHONON_INCLUDES})
add_definitions(-DPHONON_LIB_SONAME=\"${PHONON_LIB_SONAME}\")
# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_phonon\")


find_package(PulseAudio 0.9.16)
set_package_properties(PulseAudio PROPERTIES DESCRIPTION "PulseAudio Audio Server"
                       URL "http://www.pulseaudio.org/"
                       TYPE OPTIONAL
                       PURPOSE "libpulse is needed for audio setup GUI"
                      )

find_package(GLIB2)
set_package_properties(GLIB2 PROPERTIES DESCRIPTION "Low-level core library for data structure handling, portability wrappers, etc."
                       URL "http://www.gtk.org"
                       TYPE OPTIONAL
                      )

pkg_check_modules(CANBERRA libcanberra)
add_feature_info ("Audio setup GUI" CANBERRA_FOUND
                  "libcanberra is needed for audio setup GUI
                  * http://0pointer.de/lennart/projects/libcanberra"
                 )

set(kcmphonon_SRCS main.cpp devicepreference.cpp backendselection.cpp)
ki18n_wrap_ui(kcmphonon_SRCS devicepreference.ui backendselection.ui)
set(kcmphonon_LIBS
    KF5::ConfigWidgets
    KF5::I18n
    KF5::IconThemes
    KF5::KCMUtils
    KF5::KIOWidgets
    ${PHONON_LIBRARY})

if(GLIB2_FOUND AND PULSEAUDIO_FOUND AND CANBERRA_FOUND)
  add_definitions(-DHAVE_PULSEAUDIO)

  set(kcmphonon_SRCS ${kcmphonon_SRCS} audiosetup.cpp testspeakerwidget.cpp)
  ki18n_wrap_ui(kcmphonon_SRCS audiosetup.ui)

  include_directories(${GLIB2_INCLUDE_DIR} ${PULSEAUDIO_INCLUDE_DIR} ${CANBERRA_INCLUDE_DIRS})

  set(kcmphonon_LIBS ${kcmphonon_LIBS} ${GLIB2_LIBRARIES} ${PULSEAUDIO_LIBRARY} ${PULSEAUDIO_MAINLOOP_LIBRARY} ${CANBERRA_LIBRARIES})
endif()

add_library(kcm_phonon MODULE ${kcmphonon_SRCS})
target_link_libraries(kcm_phonon ${kcmphonon_LIBS} KF5::ConfigWidgets)

install(TARGETS kcm_phonon  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############
install( FILES kcm_phonon.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install(FILES listview-background.png DESTINATION ${DATA_INSTALL_DIR}/kcm_phonon)
