/* This file is part of the KDE Project
   Copyright (c) 2001 Malte Starostik <malte@kde.org>
   Copyright (c) 2006 Matthias Kretz <kretz@kde.org>

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

#include <phonon/mediaobject.h>
#include <phonon/backendcapabilities.h>
#include <kdebug.h>

#include "konq_sound.h"
#include <kurl.h>
#include <kpluginfactory.h>

using namespace std;

class KonqSoundPlayerImpl : public KonqSoundPlayer
{
public:
	KonqSoundPlayerImpl();
	virtual ~KonqSoundPlayerImpl() {}

	virtual bool isMimeTypeKnown(const QString& mimeType);
	virtual void setUrl(const KUrl &url);
	virtual void play();
	virtual void stop();
	virtual bool isPlaying();

private:
	Phonon::MediaObject *m_player;
};

KonqSoundPlayerImpl::KonqSoundPlayerImpl()
	: m_player(0)
{
}

bool KonqSoundPlayerImpl::isMimeTypeKnown(const QString& mimeType)
{
	kDebug() << mimeType << Phonon::BackendCapabilities::isMimeTypeAvailable(mimeType);
	return Phonon::BackendCapabilities::isMimeTypeAvailable(mimeType);
}

void KonqSoundPlayerImpl::setUrl(const KUrl &url)
{
	kDebug() ;
	if (!m_player) {
		kDebug() << "create AudioPlayer";
		m_player = Phonon::createPlayer(Phonon::MusicCategory);
		m_player->setParent(this);
	}
	m_player->setCurrentSource(url);
}

void KonqSoundPlayerImpl::play()
{
	kDebug() ;
	if (m_player)
		m_player->play();
}

void KonqSoundPlayerImpl::stop()
{
	kDebug() ;
	if (m_player)
		m_player->stop();
}

bool KonqSoundPlayerImpl::isPlaying()
{
	if (m_player) {
		const bool isPlaying = (m_player->state() == Phonon::PlayingState || m_player->state() == Phonon::BufferingState);
		kDebug() << isPlaying;
		return isPlaying;
	}
	kDebug() << false;
	return false;
}

class KonqSoundFactory : public KPluginFactory
{
public:
	KonqSoundFactory(QObject *parent = 0)
		: KPluginFactory(0, 0, parent) {}
	virtual ~KonqSoundFactory() {}

protected:
	virtual QObject *create(const char *iface,
							QWidget *parentWidget,
							QObject *parent,
							const QVariantList &args,
							const QString &keyword);
};

QObject *KonqSoundFactory::create(const char *iface, QWidget *, QObject *, const QVariantList &, const QString &)
{
	if (qstrcmp(iface, "KonqSoundPlayer") == 0)
		return new KonqSoundPlayerImpl();
	return 0;
}

K_EXPORT_PLUGIN(KonqSoundFactory)

// vim: ts=4 sw=4 noet
