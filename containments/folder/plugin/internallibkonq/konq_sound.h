/* This file is part of the KDE Project
   Copyright (c) 2001 Malte Starostik <malte@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __konq_sound_h__
#define __konq_sound_h__

#include <qobject.h>

class KUrl;

class KonqSoundPlayer : public QObject
{
	Q_OBJECT

public:
	virtual bool isMimeTypeKnown(const QString& mimeType) = 0;
	virtual void setUrl(const KUrl &url) = 0;
	virtual void play() = 0;
	virtual void stop() = 0;
	virtual bool isPlaying() = 0;
};

#endif

// vim: ts=4 sw=4 noet
