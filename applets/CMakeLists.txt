add_subdirectory(kicker)
add_subdirectory(kickoff)
add_subdirectory(trash)

add_subdirectory(taskmanager)
add_subdirectory(icontasks)
plasma_install_package(window-list org.kde.plasma.windowlist)

if(KF5Activities_FOUND)
   add_subdirectory(pager)
   add_subdirectory(showActivityManager)
endif()

add_subdirectory(kimpanel)
