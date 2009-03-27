#!/usr/bin/python

# Function: Convert windows' .mb table file to fcitx format
#           This version ONLY accept pure table data (i.e. 
#           no description, no rules, only table map). You
#           have to add header description after convertion
#           by hand, then use txt2mb to get you .mb file
# 
# NOTE: tables from windows IME must first be converted to
# unix text format by using dos2unix command!
#
# Author: linming04@gmail.com

import sys
import os

if len(sys.argv) != 2 :
	print "usage: winmb2fcitx mbfile.mb"
	print "NOTE: tables from windows IME must first be converted to unix text format by using dos2unix command!"
	sys.exit()

infilename = sys.argv[1]
outfilename = "outfcitx.txt"

infile = open(infilename,'r')
outfile = open(outfilename,'w')

lines = infile.readlines()
infile.close()

for string in lines:
	for i in range(0,len(string)-1,2):
		if (ord(string[i]) <=127): #this char must be not chinese char
			t0 = string[0:i]
			t1 = string[i:]
			break
	t = t1.split()
	for word in t[0:] :
		outfile.write(word + ' ' + t0 + '\n')
	#end for
#end for

outfile.close()
