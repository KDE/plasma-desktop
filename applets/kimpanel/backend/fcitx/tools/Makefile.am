DEFS = -DPKGDATADIR=\"$(pkgdatadir)\"

toolsdir = $(pkgdatadir)/tools
INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/lib
tools_DATA = winmb2fcitx.py
EXTRA_DIST = $(tools_DATA)

bin_PROGRAMS = mb2txt txt2mb createPYMB jd2fcitx pyjj2fcitx win2fcitx readPYMB readPYBase mb2org
readPYMB_SOURCES = pyTools.c pyTools.h readPYMB.c
readPYBase_SOURCES = pyTools.c pyTools.h readPYBase.c

createPYMB_LDADD = ../src/pyParser.o ../src/pyMapTable.o ../src/PYFA.o ../src/sp.o
pyjj2fcitx_LDADD = ../src/pyParser.o ../src/pyMapTable.o ../src/PYFA.o ../src/sp.o
mb2org_LDADD = ../src/pyParser.o ../src/pyMapTable.o ../src/PYFA.o ../src/sp.o pyTools.o
