var kickoffConfig = ConfigFile("kickoffrc");

kickoffConfig.group = "Favorites";
var favorites = kickoffConfig.readEntry("FavoriteURLs");
var haveFavorites = (favorites.length > 0);

if (haveFavorites) {
    favorites = favorites.split(',');
}

kickoffConfig.group = "SystemApplications";
var systemApplications = kickoffConfig.readEntry("DesktopFiles");
var haveSystemApplications = (systemApplications.length > 0);

if (haveSystemApplications) {
    systemApplications = systemApplications.split(',');

    // This used to be hardcoded in Kickoff C++ code; it's now the KConfigXT
    // default but needs to be added in when migrating from the rc file.
    systemApplications.unshift("systemsettings.desktop");
}

if (haveFavorites || haveSystemApplications) {
    for (var i in panels()) {
        var panel = panels()[i];

        for (var j in panel.widgetIds) {
            var widget = panel.widgetById(panel.widgetIds[j]);

            if (widget.type == "org.kde.plasma.kickoff") {
                widget.currentConfigGroup = ["General"];

                if (haveFavorites) {
                    widget.writeConfig("favorites", favorites);
                }

                if (haveSystemApplications) {
                    widget.writeConfig("systemApplications", systemApplications);
                }
            }
        }
    }

    for (var i in desktops()) {
        var desktop = desktops()[i];

        for (var j in desktop.widgetIds) {
            var widget = desktop.widgetById(desktop.widgetIds[j]);

            if (widget.type == "org.kde.plasma.kickoff") {
                widget.currentConfigGroup = ["General"];

                if (haveFavorites) {
                    widget.writeConfig("favorites", favorites);
                }

                if (haveSystemApplications) {
                    widget.writeConfig("systemApplications", systemApplications);
                }
            }
        }
    }
}
