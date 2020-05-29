
function swapWidget(desktop, oldWidget, newType, geometry) {
    oldWidget.remove();
    desktop.addWidget(newType, geometry.x, geometry.y, geometry.width, geometry.height);
}

for (var i in desktops()) {
    var desktop = desktops()[i];

    for (var j in desktop.widgetIds) {
        var widget = desktop.widgetById(desktop.widgetIds[j]);

        let newType = ""
        if (widget.type == "org.kde.plasma.systemloadviewer") {
            let geometry = widget.geometry;
            geometry.width = geometry.width/3
            
            widget.remove();
            desktop.addWidget("org.kde.ksysguard.sensorchart.cpuusage", geometry.x, geometry.y, geometry.width, geometry.height);
            geometry.x += geometry.width;
            desktop.addWidget("org.kde.ksysguard.sensorchart.memoryusage", geometry.x, geometry.y, geometry.width, geometry.height);
            geometry.x += geometry.width;

            let swapWidget = desktop.addWidget("org.kde.ksysguard.sensorchart", geometry.x, geometry.y, geometry.width, geometry.height);
            swapWidget.currentConfigGroup = ["Appearance"];
            swapWidget.writeConfig("title", "Swap");
            swapWidget.currentConfigGroup = ["Sensors"];
            swapWidget.writeConfig("highPrioritySensorIds", "[\"mem/swap/used\",\"mem/swap/free\"]");
            swapWidget.writeConfig("totalSensors", "[\"mem/swap/used\"]");
            swapWidget.currentConfigGroup = ["SensorColors"];
            swapWidget.writeConfig("mem/swap/free", "230,230,230");

            swapWidget.reloadconfiguration(); 
        }
    }
}

