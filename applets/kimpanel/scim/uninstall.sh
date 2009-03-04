#! /bin/sh

pkill scim
pkill -9 scim
sudo cp ./scim-panel-dbus /usr/lib/scim-1.0/ -v
rm ~/a.log
