/*
   Copyright (C) 2004 George Staikos <staikos@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#include <k4aboutdata.h>
#include <KApplication>
#include <KCmdLineArgs>
#include <KLocalizedString>

#include "knetattach.h"

int main(int argc, char **argv) {
    KLocalizedString::setApplicationDomain("knetattach");

	K4AboutData about("knetattach", 0, ki18n("KDE Network Wizard"), "1.0",
		ki18n("KDE Network Wizard"),
		K4AboutData::License_GPL,
		ki18n("(c) 2004 George Staikos"), KLocalizedString(),
		"http://www.kde.org/");

	about.addAuthor(ki18n("George Staikos"), ki18n("Primary author and maintainer"), "staikos@kde.org");

	KCmdLineArgs::init(argc, argv, &about);
	KApplication a;

	KNetAttach na;
    a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
	na.show();

	return a.exec();
}
