#! /bin/sh

pkill scim
pkill -9 scim
rm -rf ~/.scim
sudo cp /home/ora/build/kimpanel/scim/scim-panel-dbus /usr/lib/scim-1.0/ -v
rm ~/a.log
