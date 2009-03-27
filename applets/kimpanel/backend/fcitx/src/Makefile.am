DEFS = -DPKGDATADIR=\"$(pkgdatadir)\"

INCLUDES = -I. -I$(top_srcdir)/lib -I$(top_srcdir)/xpm
AM_CFLAGS = -O2 -fno-strength-reduce


bin_PROGRAMS = fcitx

fcitx_SOURCES = IC.c IC.h \
                ime.c ime.h \
                InputWindow.c InputWindow.h \
                KeyList.c KeyList.h \
                main.c main.h \
                MainWindow.c MainWindow.h \
                MyErrorsHandlers.c MyErrorsHandlers.h \
                punc.c punc.h \
                py.c py.h \
                PYFA.c PYFA.h \
                pyMapTable.c pyMapTable.h \
                pyParser.c pyParser.h \
                sp.c sp.h \
                tools.c tools.h \
                ui.c ui.h \
                version.h \
                table.c table.h \
		DBus.c DBus.h \
                xim.c xim.h \
                qw.c qw.h \
		vk.c vk.h \
		about.c about.h \
		QuickPhrase.c QuickPhrase.h \
		AutoEng.c AutoEng.h \
		fcitx.h extra.c extra.h \
		internalVersion.c
		
fcitx_LDADD = ../lib/libXimd.a
fcitx_LDFLAGS = -lXpm
