/* This file is part of the KDE project
   Copyright (C) 2000      Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2000-2007 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __konq_events_h__
#define __konq_events_h__

#include <kparts/event.h>
#include <QList>
#include <libkonq_export.h>
#include <kfileitem.h>
#include <kconfigbase.h>

namespace KParts
{
  class ReadOnlyPart;
}

class KConfig;

class LIBKONQ_EXPORT KonqFileSelectionEvent : public KParts::Event
{
public:
  KonqFileSelectionEvent( const KFileItemList&selection, KParts::ReadOnlyPart *part ) : KParts::Event( s_fileItemSelectionEventName ), m_selection( selection ), m_part( part ) {}

  KFileItemList selection() const { return m_selection; }
  KParts::ReadOnlyPart *part() const { return m_part; }

  static bool test( const QEvent *event ) { return KParts::Event::test( event, s_fileItemSelectionEventName ); }

private:
  static const char * const s_fileItemSelectionEventName;

  KFileItemList m_selection;
  KParts::ReadOnlyPart *m_part;
};

class LIBKONQ_EXPORT KonqFileMouseOverEvent : public KParts::Event
{
public:
  KonqFileMouseOverEvent( const KFileItem& item, KParts::ReadOnlyPart *part ) : KParts::Event( s_fileItemMouseOverEventName ), m_item( item ), m_part( part ) {}

  const KFileItem& item() const { return m_item; }
  KParts::ReadOnlyPart *part() const { return m_part; }

  static bool test( const QEvent *event ) { return KParts::Event::test( event, s_fileItemMouseOverEventName ); }

private:
  static const char * const s_fileItemMouseOverEventName;

  KFileItem m_item;
  KParts::ReadOnlyPart *m_part;
};

class LIBKONQ_EXPORT KonqConfigEvent : public KParts::Event
{
public:
  KonqConfigEvent( KConfigBase *config, const QString &prefix, bool save ) : KParts::Event( s_configEventName ), m_config( config ), m_prefix( prefix ), m_save( save ) {}

  KConfigBase * config() const { return m_config; }
  QString prefix() const { return m_prefix; }
  bool save() const { return m_save; }

  static bool test( const QEvent *event ) { return KParts::Event::test( event, s_configEventName ); }
private:
  static const char * const s_configEventName;

  KConfigBase *m_config;
  QString m_prefix;
  bool m_save;
};

#endif
