set(fontthumbnail_PART_SRCS FontThumbnail.cpp )

ecm_qt_declare_logging_category(fontthumbnail_PART_SRCS
    HEADER debug.h
    IDENTIFIER KCM_KFONTINST_THUMBNAIL
    CATEGORY_NAME org.kde.kcm_kfontinst.thumbnail)

add_library(fontthumbnail MODULE ${fontthumbnail_PART_SRCS})

target_link_libraries(fontthumbnail KF5::Archive KF5::KIOCore KF5::KIOWidgets kfontinstui kfontinst )

install(TARGETS fontthumbnail  DESTINATION ${KDE_INSTALL_PLUGINDIR} )
install( FILES fontthumbnail.desktop  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR} )

