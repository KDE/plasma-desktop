find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Test)

include(ECMAddTests)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

ecm_add_tests(
   screenmappertest.cpp
   foldermodeltest.cpp 
   positionertest.cpp
   viewpropertiesmenutest.cpp
   LINK_LIBRARIES Qt::Test folderplugin
)


