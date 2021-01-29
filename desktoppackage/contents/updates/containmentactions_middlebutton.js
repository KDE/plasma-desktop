// MidButton got deprecated and doesn't really work anymore as a stringified enum value
// for the middle button  Qt::MouseButton, we need to update our config to "MiddleButton"
var plasmaConfig = ConfigFile("plasma-org.kde.plasma.desktop-appletsrc", "ActionPlugins");

for (let i in plasmaConfig.groupList) {
    let subGroup = ConfigFile(plasmaConfig, plasmaConfig.groupList[i])

    for (let j in subGroup.keyList) {
        let key = subGroup.keyList[j];
        if (key.indexOf("MidButton") !== -1) {
            subGroup.writeEntry(key.replace("MidButton", "MiddleButton"), subGroup.readEntry(key));
            subGroup.deleteEntry(key);
        }
    }
}
