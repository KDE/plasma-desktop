#!/bin/bash

current_directory=`pwd`

if [ -z "$1" ]; then
    echo "You need to pass the path to the kactivities directory"
    exit 1
fi

kactivities_directory=$1

pushd $1

project_name="`git remote show origin | grep Push | sed 's/.*\///'`"
if [ "$project_name" != "kactivities" ]; then
    echo "You need to pass the path to the kactivities directory, not for $project_name"
    exit 1
fi

kactivities_branch="`git rev-parse --abbrev-ref HEAD`"
if [ "$2" != "--force-branch" -a "$kactivities_branch" != "master" ]; then
    echo "The branch of kactivities needs to be master"
    echo "Alternatively, you could call $0 $1 --force-branch"
    exit 1
fi

popd

echo "will run: rm -fr ./src/lib/stats"
echo "will run: rm -fr ./src/common"
echo "will run: rm -fr ./src/utils"
echo "[enter] to continue, [ctrl+c] to cancel"
read

rm -fr ./src/lib/stats
rm -fr ./src/common
rm -fr ./src/utils

echo "will run: cp -a $kactivities_directory/src/lib/stats ./src/lib/stats"
echo "will run: cp -a $kactivities_directory/src/common    ./src/common"
echo "will run: cp -a $kactivities_directory/src/utils     ./src/utils"
echo "[enter] to continue, [ctrl+c] to cancel"
read

echo "copying files from kactivities"
cp -a $kactivities_directory/src/lib/stats ./src/lib/stats
cp -a $kactivities_directory/src/common    ./src/common
cp -a $kactivities_directory/src/utils     ./src/utils

