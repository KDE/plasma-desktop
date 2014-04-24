#add_subdirectory( pics )
#add_subdirectory( favicons )
#add_subdirectory( Templates )
#add_subdirectory( tests )

########### libkonq ###############

# For crc32 in konq_historyloader.cpp
include_directories( ${ZLIB_INCLUDE_DIR} )

set(konq_LIB_SRCS
   konq_popupmenu.cpp       # used by konqueror, kfind, folderview, kickoff
   konq_popupmenuplugin.cpp # for KonqPopupMenu and its plugins
   konq_dndpopupmenuplugin.cpp # for KonqDndPopupMenu and its plugins
   konq_copytomenu.cpp         # used by dolphin, KonqPopupMenu
   konq_operations.cpp         # used by dolphin and konqueror
   konq_statusbarmessagelabel.cpp  # used by dolphin and konqueror
   konq_events.cpp
   konqmimedata.cpp         # used by dolphin, KonqOperations, some filemanagement konqueror modules.
   konq_historyentry.cpp
   konq_historyloader.cpp
   konq_historyprovider.cpp
   kversioncontrolplugin.cpp  # used by dolphin and its version control plugins (deprecated)
   kversioncontrolplugin2.cpp # used by dolphin and its version control plugins

#    konq_nameandurlinputdialog.cpp # deprecated (functionality has moved to kdelibs)
   knewmenu.cpp # deprecated (functionality has moved to kdelibs)
   konq_popupmenuinformation.cpp # deprecated (functionality has moved to kdelibs)
   konq_menuactions.cpp     # deprecated (functionality has moved to kdelibs)
   konq_fileitemcapabilities.cpp # deprecated (functionality has moved to kdelibs)
)

add_library(konq SHARED ${konq_LIB_SRCS})

target_link_libraries(konq KF5::Bookmarks KF5::KDELibs4Support KF5::Parts ${ZLIB_LIBRARY} ${X11_X11_LIB})

set_target_properties(konq PROPERTIES
     VERSION ${KDE_NON_GENERIC_LIB_VERSION}
     SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION}
)

install(TARGETS konq  ${INSTALL_TARGETS_DEFAULT_ARGS})

########### konq_sound ###############

# TODO: bring back
# set(konq_sound_LIB_SRCS konq_sound.cc)
# add_library(konq_sound MODULE ${konq_sound_LIB_SRCS})
# target_link_libraries(konq_sound  ${KDE4_PHONON_LIBS})
# install(TARGETS konq_sound  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

# install( FILES directory_bookmarkbar.desktop   DESTINATION  ${DATA_INSTALL_DIR}/kbookmark )
# install( FILES
#  konq_popupmenu.h
#  konq_popupmenuinformation.h
#  konq_popupmenuplugin.h
#  konq_dndpopupmenuplugin.h
#  knewmenu.h
#  konq_menuactions.h
#  # konq_copytomenu.h - anyone needs it?
#  konq_operations.h
#  libkonq_export.h
#  konq_events.h
#  konqmimedata.h
#  konq_fileitemcapabilities.h
#  kversioncontrolplugin.h
#  kversioncontrolplugin2.h
#  konq_historyprovider.h
#  konq_historyentry.h
# DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
# )
# install( FILES konqpopupmenuplugin.desktop konqdndpopupmenuplugin.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
