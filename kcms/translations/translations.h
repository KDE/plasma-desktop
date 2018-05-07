/*
 *  Copyright (C) 2014 John Layt <john@layt.net>
 *  Copyright (C) 2018 Eike Hein <hein@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef TRANSLATIONS_H
#define TRANSLATIONS_H

#include <KQuickAddons/ConfigModule>

#include <KConfigGroup>

class TranslationsModel;

class QAbstractListModel;

class Translations : public KQuickAddons::ConfigModule
{
    Q_OBJECT

    Q_PROPERTY(QAbstractItemModel* translationsModel READ translationsModel CONSTANT)
    Q_PROPERTY(bool everSaved READ everSaved NOTIFY everSavedChanged)

    public:
        explicit Translations(QObject* parent = 0, const QVariantList &list = QVariantList());
        ~Translations() override;

        QAbstractItemModel* translationsModel() const;

        bool everSaved() const;

    public Q_SLOTS:
        void load();
        void save();
        void defaults();

    Q_SIGNALS:
        void everSavedChanged() const;

    private Q_SLOTS:
        void selectedLanguagesChanged();
        void missingLanguagesChanged();

    private:
        TranslationsModel *m_translationsModel;

        KConfigGroup m_config;
        QStringList m_configuredLanguages;

        bool m_everSaved;
};

#endif
