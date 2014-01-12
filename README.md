Touchpad KCM
============

* KCModule
* Daemon
  - Automatically enable/disable touchpad when typing and/or when mouse is plugged in
  - Enable/disable touchpad with keyboard shortcuts
* Applet
  - Shows touchpad's state
  - Toggle touchpad with single click

Dependencies
------------

* KDElibs 4
* Xlib
* xcb
* xf86-input-synaptics (headers are required to build)


How to install
--------------

    $ mkdir build
    $ cd build
    $ cmake -DCMAKE_INSTALL_PREFIX="$(kde4-config --prefix)" ..
    $ make
    # make install

Also, there is a package for Ubuntu in [Kubuntu Experimental PPA](https://launchpad.net/~kubuntu-ppa/+archive/experimental)

Bugs
----

Bugs should be reported to [KDE Bugzilla, product: Touchpad KCM](https://bugs.kde.org/enter_bug.cgi?product=Touchpad%20KCM)
