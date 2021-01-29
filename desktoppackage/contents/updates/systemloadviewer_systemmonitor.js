
function swapWidget(cont, oldWidget, newType, geometry) {
    oldWidget.remove();
    cont.addWidget(newType, geometry.x, geometry.y, geometry.width, geometry.height);
}

var containments = desktops().concat(panels());

for (var i in containments) {
    var cont = containments[i];

    for (var j in cont.widgetIds) {
        var widget = cont.widgetById(cont.widgetIds[j]);

        let newType = ""
        if (widget.type == "org.kde.plasma.systemloadviewer") {
            let geometry = widget.geometry;
            geometry.width = geometry.width/3
            
            widget.remove();
            cont.addWidget("org.kde.plasma.systemmonitor.cpuusage", geometry.x, geometry.y, geometry.width, geometry.height);
            geometry.x += geometry.width;
            cont.addWidget("org.kde.plasma.systemmonitor.memoryusage", geometry.x, geometry.y, geometry.width, geometry.height);
            geometry.x += geometry.width;

            let swapWidget = cont.addWidget("org.kde.plasma.systemmonitor", geometry.x, geometry.y, geometry.width, geometry.height);
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

