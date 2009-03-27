#!/bin/sh

aclocal || exit 1
autoheader || exit 2
automake --add-missing --copy --include-deps || exit 3
autoconf || exit 4

if [ -z "$1" ] ; then
  echo
  echo 'FCITX now prepared to build. Run:'
  echo " ./configure && make"
else
  ./configure "$@" || exit 4
  make || exit 5
fi

echo
echo 'Build and Install on system:'
echo '  ./configure && make install   # run as root'

echo 'Build and Install on user home:'
echo '  ./configure --prefix="$HOME" && make install'

echo 'Build without xft:'
echo '  ./configure --enable-xft=no && make'

echo 'Build without dbus:'
echo '  ./configure --enable-dbus=no && make'

echo
echo 'Build RPM Package:'
echo '  ./configure && make dist'
echo '  rpmbuild -ts fcitx-*.tar.gz   # build source package'
echo '  rpmbuild -tb fcitx-*.tar.gz   # build binary package'

echo
echo 'Build Debian Package:'
echo '  ./configure && fakeroot dpkg-buildpackage -uc -b -d'

echo
echo 'Create and test source distribution:'
echo '  ./configure && make distcheck'

