include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

ecm_add_tests(
   screenmappertest.cpp
   foldermodeltest.cpp 
   positionertest.cpp
   viewpropertiesmenutest.cpp
   LINK_LIBRARIES Qt::Test folderplugin
)


