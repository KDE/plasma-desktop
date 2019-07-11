var allDesktops = desktops();

for (var i = 0; i < allDesktops.length; ++i) {
    var desktop = allDesktops[i];
    desktop.currentConfigGroup = ["General"];
    var serializedItems = desktop.readConfig("ItemsGeometries");
    desktop.currentConfigGroup = [];
    desktop.writeConfig("ItemGeometriesHorizontal", serializedItems);
    desktop.reloadConfig()
}
