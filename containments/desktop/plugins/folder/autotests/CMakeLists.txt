include_directories(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/..)

add_library(foldertest STATIC
   ../foldermodel.cpp
   ../viewpropertiesmenu.cpp
   ../screenmapper.cpp
   ../removeaction.cpp
   ../positioner.cpp
   ../itemviewadapter.cpp
   ../desktopschemehelper.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/../debug.cpp
)

target_link_libraries(foldertest PUBLIC Qt::Gui Qt::Quick Plasma::Plasma KF6::KIOFileWidgets KF6::XmlGui KF6::I18n Plasma::Activities KF6::Notifications)

ecm_add_tests(
   desktopschemehelpertest.cpp
   screenmappertest.cpp
   foldermodeltest.cpp
   positionertest.cpp
   viewpropertiesmenutest.cpp
   LINK_LIBRARIES Qt::Test foldertest
)


