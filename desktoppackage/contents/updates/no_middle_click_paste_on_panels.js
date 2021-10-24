// In the past, panels were configured to add a note on middle-click. This was changed,
// but the "MiddleButton;NoModifier=org.kde.paste" action was never removed from the
// config file, so some people still got this undesirable behavior with no GIU method
// to change it.
//
// This script removes it.

var plasmaConfig = ConfigFile("plasma-org.kde.plasma.desktop-appletsrc", "ActionPlugins");

for (let i in plasmaConfig.groupList) {
    let subSubGroupKeys = [];
    let subGroup = ConfigFile(plasmaConfig, plasmaConfig.groupList[i]);
    for (let j in subGroup.groupList) {
        let subSubGroup = ConfigFile(subGroup, subGroup.groupList[j]);
        subSubGroupKeys = subSubGroup.keyList;
    }
    if (subSubGroupKeys.indexOf("_sep1") === -1) {
        print("Containment " + i + " Does not have a _sep1 item; it must be a panel.\n");
        // No _sep1 item; this must be a panel
        let mmbAction = subGroup.readEntry("MiddleButton;NoModifier");
        if (mmbAction === "org.kde.paste") {
            print("Panel " + i + " Seems to have a middle-click paste action defined; deleting it!");
            subGroup.deleteEntry("MiddleButton;NoModifier");
        }
    }
}
