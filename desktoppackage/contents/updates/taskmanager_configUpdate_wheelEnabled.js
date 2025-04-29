// Update all applets with wheelEnabled=true to wheelEnabled=AllTask

var containments = desktops().concat(panels());

containments.forEach(function(cont) {

    cont.widgetIds.forEach(function(id) {
        var widget = cont.widgetById(id);

        widget.currentConfigGroup = new Array("General");

        if (widget.readConfig("wheelEnabled", false) === true) {
            widget.writeConfig("wheelEnabled", "AllTask");
            widget.reloadConfig();
        }
    });
});
