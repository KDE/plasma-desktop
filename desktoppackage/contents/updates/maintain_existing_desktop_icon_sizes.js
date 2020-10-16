// This script updates users' Folder View icon sizes following a change in what
// they mean in https://invent.kde.org/plasma/plasma-desktop/-/merge_requests/111

var allDesktops = desktops();

for (var i = 0; i < allDesktops.length; ++i) {
    var desktop = allDesktops[i];
    desktop.currentConfigGroup = ["General"];

    var currentIconSize = desktop.readConfig("iconSize");

    // Don't do anything if there is no value in the config file, since in this
    // case, no change is needed because the new default works properly
    if (currentIconSize) {
        currentIconSize = parseInt(currentIconSize);

        // No change needed for iconSize=0 or 5
        if (currentIconSize != 0 && currentIconSize != 5) {
            print("Current icon size is " + currentIconSize);
            var newIconSize = 3;

            switch(currentIconSize) {
                case 1:
                    newIconSize = 0;
                    break;
                case 2:
                    newIconSize = 1;
                    break;
                case 3:
                    newIconSize = 2;
                    break;
                case 4:
                    newIconSize = 3;
                    break;
                // We should never reach here, but in case we do anyway, reset to
                // the default value
                default:
                    break;
            }

            desktop.writeConfig("iconSize", newIconSize);
            desktop.reloadConfig()
        }
    }
}

