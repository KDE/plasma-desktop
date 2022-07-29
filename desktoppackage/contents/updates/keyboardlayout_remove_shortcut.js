const containments = desktops().concat(panels());
for (var i in containments) {
    forEachWidgetInContainment(containments[i]);
}

function forEachWidgetInContainment(containment) {
    const widgets = containment.widgets();
    for (var i in widgets) {
        const widget = widgets[i];
        switch(widget.type) {
        case "org.kde.plasma.systemtray":
            systemtrayId = widget.readConfig("SystrayContainmentId");
            if (systemtrayId) {
                forEachWidgetInContainment(desktopById(systemtrayId))
            }
            break;
        case "org.kde.plasma.keyboardlayout":
            if (widget.globalShortcut) {
                print("Shortcut to remove: " + widget.globalShortcut);
                widget.globalShortcut = "";
            }
            break;
        }
    }
}
