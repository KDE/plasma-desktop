
include_directories(
    ${kcms_SOURCE_DIR}/kfontinst/viewpart/
     )

set(fontthumbnail_PART_SRCS FontThumbnail.cpp )

add_library(fontthumbnail MODULE ${fontthumbnail_PART_SRCS})

target_link_libraries(fontthumbnail KF5::Archive KF5::KIOCore KF5::KIOWidgets kfontinstui kfontinst )

install(TARGETS fontthumbnail  DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES fontthumbnail.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

