#!/bin/sh
#
# Note: This file is tkaen, and modified, from gucharmap - svn revision 1169
#
# usage: ./download-unicode-files.sh DIRECTORY
# downloads following files from unicode.org to DIRECTORY or unicode/ (if
# DIRECTORY is not presented):
#  - UnicodeData.txt
#  - Unihan.zip
#  - NamesList.txt
#  - Blocks.txt
#  - Scripts.txt
#

# FILES='UnicodeData.txt Unihan.zip NamesList.txt Blocks.txt Scripts.txt'

FILES='UnicodeData.txt Blocks.txt Scripts.txt'

mkdir -p ${1:-unicode} 

for x in $FILES; do
	wget "https://www.unicode.org/Public/UNIDATA/$x" -O "${1:-unicode}/$x"
done

echo 'Done.'

