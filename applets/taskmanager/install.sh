#!/bin/sh
kpackagetool5 -t Plasma/Applet --install package || kpackagetool5 -t Plasma/Applet --upgrade package
pkill plasmashell && plasmashell --replace &
 
