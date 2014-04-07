#! /usr/bin/env bash
$EXTRACTRC *.ui >> rc.cpp
find . -type d | fgrep -v '.svn' | sed -e 's,$,/,' > dirs
msh=`find . -name Messages.sh`
for dir in $msh; do
  dir=`dirname $dir`
  if test "$dir" != "."; then
    egrep -v "^$dir" dirs > dirs.new && mv dirs.new dirs
  fi
done
fgrep -v "/tests" dirs > dirs.new && mv dirs.new dirs
dirs=`cat dirs`
find $dirs -maxdepth 1 -name "*.cpp" -print > files
find $dirs -maxdepth 1 -name "*.cc" -print >> files
find $dirs -maxdepth 1 -name "*.h" -print >> files
$XGETTEXT --files-from=files -o $podir/kcm_phonon.pot
rm -f dirs
rm -f files
