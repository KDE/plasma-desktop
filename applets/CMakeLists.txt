add_subdirectory(kicker)
add_subdirectory(kickoff)
add_subdirectory(trash)

add_subdirectory(taskmanager)
plasma_install_package(window-list org.kde.plasma.windowlist)
plasma_install_package(icontasks org.kde.plasma.icontasks)

if(KF5Activities_FOUND)
   add_subdirectory(pager)
   add_subdirectory(showActivityManager)
endif()

add_subdirectory(minimizeall)
add_subdirectory(showdesktop)
add_subdirectory(kimpanel)
