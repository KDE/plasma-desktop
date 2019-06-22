/***************************************************************************
 *   Copyright (C) 2012 Aurélien Gâteau <agateau@kde.org>                  *
 *   Copyright (C) 2013-2015 by Eike Hein <hein@kde.org>                   *
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

#ifndef APPSMODEL_H
#define APPSMODEL_H

#include "abstractmodel.h"
#include "appentry.h"

#include <QQmlParserStatus>

#include <KServiceGroup>


class QTimer;

class AppsModel : public AbstractModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(bool autoPopulate READ autoPopulate WRITE setAutoPopulate NOTIFY autoPopulateChanged)

    Q_PROPERTY(bool paginate READ paginate WRITE setPaginate NOTIFY paginateChanged)
    Q_PROPERTY(int pageSize READ pageSize WRITE setPageSize NOTIFY pageSizeChanged)
    Q_PROPERTY(bool flat READ flat WRITE setFlat NOTIFY flatChanged)
    Q_PROPERTY(bool sorted READ sorted WRITE setSorted NOTIFY sortedChanged)
    Q_PROPERTY(bool showSeparators READ showSeparators WRITE setShowSeparators NOTIFY showSeparatorsChanged)
    Q_PROPERTY(bool showTopLevelItems READ showTopLevelItems WRITE setShowTopLevelItems NOTIFY showTopLevelItemsChanged)
    Q_PROPERTY(int appNameFormat READ appNameFormat WRITE setAppNameFormat NOTIFY appNameFormatChanged)
    Q_PROPERTY(QObject* appletInterface READ appletInterface WRITE setAppletInterface NOTIFY appletInterfaceChanged)

    public:
        explicit AppsModel(const QString &entryPath = QString(), bool paginate = false, int pageSize = 24,
            bool flat = false, bool sorted = true, bool separators = true, QObject *parent = nullptr);
        explicit AppsModel(const QList<AbstractEntry *> entryList, bool deleteEntriesOnDestruction, QObject *parent = nullptr);
        ~AppsModel() override;

        QString description() const override;
        void setDescription(const QString &text);

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

        int rowCount(const QModelIndex &parent = QModelIndex()) const override;

        Q_INVOKABLE bool trigger(int row, const QString &actionId, const QVariant &argument) override;

        bool autoPopulate() const;
        void setAutoPopulate(bool populate);

        Q_INVOKABLE AbstractModel *modelForRow(int row) override;
        Q_INVOKABLE int rowForModel(AbstractModel *model) override;

        int separatorCount() const override;

        bool paginate() const;
        void setPaginate(bool paginate);

        int pageSize() const;
        void setPageSize(int size);

        bool flat() const;
        void setFlat(bool flat);

        bool sorted() const;
        void setSorted(bool sorted);

        bool showSeparators() const;
        void setShowSeparators(bool showSeparators);

        bool showTopLevelItems() const;
        void setShowTopLevelItems(bool showTopLevelItems);

        int appNameFormat() const;
        void setAppNameFormat(int format);

        QObject *appletInterface() const;
        void setAppletInterface(QObject *appletInterface);

        QStringList hiddenEntries() const;

        void entryChanged(AbstractEntry *entry) override;

        void classBegin() override;
        void componentComplete() override;

    Q_SIGNALS:
        void cleared() const;
        void autoPopulateChanged() const;
        void paginateChanged() const;
        void pageSizeChanged() const;
        void flatChanged() const;
        void sortedChanged() const;
        void showSeparatorsChanged() const;
        void showTopLevelItemsChanged() const;
        void appNameFormatChanged() const;
        void appletInterfaceChanged() const;
        void hiddenEntriesChanged() const;

    protected Q_SLOTS:
        void refresh() override;

    protected:
        void refreshInternal();

        bool m_complete;

        bool m_paginate;
        int m_pageSize;

        QList<AbstractEntry *> m_entryList;
        bool m_deleteEntriesOnDestruction;
        int m_separatorCount;
        bool m_showSeparators;
        bool m_showTopLevelItems;

        QObject *m_appletInterface;

    private Q_SLOTS:
        void checkSycocaChanges(const QStringList &changes);

    private:
        void processServiceGroup(KServiceGroup::Ptr group);
        void sortEntries();

        bool m_autoPopulate;

        QString m_description;
        QString m_entryPath;
        bool m_staticEntryList;
        QTimer *m_changeTimer;
        bool m_flat;
        bool m_sorted;
        AppEntry::NameFormat m_appNameFormat;
        QStringList m_hiddenEntries;
        static MenuEntryEditor *m_menuEntryEditor;
};

#endif
