// Find all digital clock applets in all containments and change
// displayTimezoneAsCode=false
// to
// displayTimezoneFormat=FullText
// See https://invent.kde.org/plasma/plasma-workspace/-/merge_requests/751

var containments = desktops().concat(panels());
for (var i in containments) {
    var cont = containments[i];

    for (var j in cont.widgetIds) {
        var widget = cont.widgetById(cont.widgetIds[j]);

        if (widget.type == "org.kde.plasma.digitalclock") {
            widget.currentConfigGroup = new Array('Appearance')
            if (widget.readConfig("displayTimezoneAsCode", true) == false) {
                widget.writeConfig("displayTimezoneFormat", "FullText")
                // Work around not being able to delete config file keys using widget interface
                widget.writeConfig("displayTimezoneAsCode", "")
            }
        }
    }
}
