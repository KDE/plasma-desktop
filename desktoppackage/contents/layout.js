
loadTemplate("org.kde.plasma.desktop.defaultPanel")

for (var i = 0; i < screenCount; ++i) {
    var id = createActivity("Desktop");
    var desktopsArray = desktopsForActivity(id);
    print(desktopsArray.length);
    for( var j = 0; j < desktopsArray.length; j++) {
        desktopsArray[j].wallpaperPlugin = 'org.kde.image';
        //var clock = desktopsArray[j].addWidget("org.kde.plasma.analogclock");
    }
}
