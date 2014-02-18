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

Translations
------------
Translations live in KDE's svn repository.
If they aren't shipped as part of KDE l10n packages, translations could be built and installed by adding -DTRANSLATIONS="lang1 lang2 ..." to cmake command line.
Language names are the same as in .desktop files.
.po files are fetched from svn repository automatically.
If downloading isn't possible, .po files could be placed in source directory under names "kcm_touchpad_lang1.po" and "plasma_applet_touchpad_lang1.po", where "lang1" is replaced with proper language name.

Bugs
----

Bugs should be reported to [KDE Bugzilla, product: Touchpad KCM](https://bugs.kde.org/enter_bug.cgi?product=Touchpad%20KCM)
