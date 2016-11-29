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

### Packages

* [Ubuntu][1]
  - [Kubuntu Experimental PPA][2]
* [Arch Linux][3]
  - [AUR (from git master)][4]


Bugs
----

Bugs should be reported to [KDE Bugzilla, product: Touchpad KCM][5]

Patches
-------
I'm always happy to accept patches at:

* [KDE reviewboard][6], repository "kcm-touchpad". The repository contains a .reviewboardrc file.
* [GitHub with pull request][7].

[1]: http://packages.ubuntu.com/trusty/kde/kde-touchpad
[2]: https://code.launchpad.net/~rohangarg/+recipe/touchpad-daily
[3]: https://www.archlinux.org/packages/community/x86_64/kcm-touchpad/
[4]: https://aur.archlinux.org/packages/kcm-touchpad-git/
[5]: https://bugs.kde.org/enter_bug.cgi?product=Touchpad%20KCM
[6]: https://git.reviewboard.kde.org/
[7]: https://github.com/sanya-m/kde-touchpad-config
