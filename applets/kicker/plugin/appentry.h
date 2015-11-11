/***************************************************************************
 *   Copyright (C) 201 by Eike Hein <hein@kde.org>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef APPENTRY_H
#define APPENTRY_H

#include "abstractentry.h"

#include <KService>
#include <KServiceGroup>

class AppsModel;
class MenuEntryEditor;

class QQmlPropertyMap;

class AppEntry : public AbstractEntry
{
    public:
        enum NameFormat {
            NameOnly = 0,
            GenericNameOnly,
            NameAndGenericName,
            GenericNameAndName
        };

        explicit AppEntry(AbstractModel *owner, KService::Ptr service, NameFormat nameFormat);
        explicit AppEntry(AbstractModel *owner, const QString &id);

        EntryType type() const { return RunnableType; }

        bool isValid() const;

        QIcon icon() const;
        QString name() const;
        QString description() const;
        KService::Ptr service() const;

        QString id() const;
        QUrl url() const;

        bool hasActions() const;
        QVariantList actions() const;

        bool run(const QString& actionId = QString(), const QVariant &argument = QVariant());

        static QString nameFromService(const KService::Ptr service, NameFormat nameFormat);
        static KService::Ptr defaultAppByName(const QString &name);

    private:
        void init(NameFormat nameFormat);

        QString m_id;
        QString m_name;
        QString m_description;
        QIcon m_icon;
        KService::Ptr m_service;
        static QObject *m_appletInterface;
        static QQmlPropertyMap *m_appletConfig;
        static MenuEntryEditor *m_menuEntryEditor;
};

class AppGroupEntry : public AbstractGroupEntry
{
    public:
        AppGroupEntry(AppsModel *parentModel, KServiceGroup::Ptr group,
            bool flat, bool separators, int appNameFormat);

        QIcon icon() const;
        QString name() const;

        bool hasChildren() const;
        AbstractModel *childModel() const;

    private:
        QString m_name;
        QIcon m_icon;
        QPointer<AbstractModel> m_childModel;
};

#endif
