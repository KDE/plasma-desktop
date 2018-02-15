include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_BINARY_DIR}
     )

set(kfontviewpart_PART_SRCS ${libkfontinstview_SRCS} ${libkfontinstdbusiface_SRCS} FontViewPart.cpp )

add_library(kfontviewpart MODULE ${kfontviewpart_PART_SRCS})

target_link_libraries(kfontviewpart KF5::Archive KF5::Parts KF5::XmlGui ${FONTCONFIG_LIBRARIES} kfontinstui kfontinst )

install(TARGETS kfontviewpart  DESTINATION ${KDE_INSTALL_PLUGINDIR} )
install( FILES kfontviewpart.rc DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/kfontinst )
install( FILES kfontviewpart.desktop  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR} )

