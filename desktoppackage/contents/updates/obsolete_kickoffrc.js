var kickoffConfig = ConfigFile("kickoffrc");

kickoffConfig.group = "Favorites";
var favorites = kickoffConfig.readEntry("FavoriteURLs").split(',');

kickoffConfig.group = "SystemApplications";
var systemApplications = kickoffConfig.readEntry("DesktopFiles").split(',');

if (systemApplications) {
    // This used to be hardcoded in Kickoff C++ code; it's now the KConfigXT
    // default but needs to be added in when migrating from the rc file.
    systemApplications.unshift("systemsettings.desktop");
}

if (favorites || systemApplications) {
    for (var i in panels()) {
        var panel = panels()[i];

        for (var j in panel.widgetIds) {
            var widget = panel.widgetById(panel.widgetIds[j]);

            if (widget.type == "org.kde.plasma.kickoff") {
                if (favorites) {
                    widget.writeConfig("favorites", favorites);
                }

                if (systemApplications) {
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
                if (favorites) {
                    widget.writeConfig("favorites", favorites);
                }

                if (systemApplications) {
                    widget.writeConfig("systemApplications", systemApplications);
                }
            }
        }
    }
}
