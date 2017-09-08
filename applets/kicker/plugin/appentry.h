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

        EntryType type() const Q_DECL_OVERRIDE { return RunnableType; }

        bool isValid() const Q_DECL_OVERRIDE;

        QIcon icon() const Q_DECL_OVERRIDE;
        QString name() const Q_DECL_OVERRIDE;
        QString description() const Q_DECL_OVERRIDE;
        KService::Ptr service() const;

        QString id() const Q_DECL_OVERRIDE;
        QUrl url() const Q_DECL_OVERRIDE;

        bool hasActions() const Q_DECL_OVERRIDE;
        QVariantList actions() const Q_DECL_OVERRIDE;

        bool run(const QString& actionId = QString(), const QVariant &argument = QVariant()) Q_DECL_OVERRIDE;

        QString menuId() const;

        static QString nameFromService(const KService::Ptr service, NameFormat nameFormat);
        static KService::Ptr defaultAppByName(const QString &name);

    private:
        void init(NameFormat nameFormat);

        QString m_id;
        QString m_name;
        QString m_description;
        mutable QIcon m_icon;
        KService::Ptr m_service;
        static MenuEntryEditor *m_menuEntryEditor;
};

class AppGroupEntry : public AbstractGroupEntry
{
    public:
        AppGroupEntry(AppsModel *parentModel, KServiceGroup::Ptr group,
            bool paginate, int pageSize, bool flat, bool sorted, bool separators, int appNameFormat);

        QIcon icon() const Q_DECL_OVERRIDE;
        QString name() const Q_DECL_OVERRIDE;

        bool hasChildren() const Q_DECL_OVERRIDE;
        AbstractModel *childModel() const Q_DECL_OVERRIDE;

    private:
        KServiceGroup::Ptr m_group;
        mutable QIcon m_icon;
        QPointer<AbstractModel> m_childModel;
};

#endif
