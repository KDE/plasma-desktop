// Find all Keyboard Layout applets in all containments and change
// showFlag=true
// to
// displayStyle=Flag
// See https://invent.kde.org/plasma/plasma-workspace/-/merge_requests/1131

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
            widget.currentConfigGroup = new Array('General')
            if (widget.readConfig("showFlag", false) == true) {
                widget.writeConfig("displayStyle", "Flag")
                // Work around not being able to delete config file keys using widget interface
                widget.writeConfig("showFlag", "")
            }
            break;
        }
    }
}
