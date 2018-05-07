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

#ifndef TRANSLATIONSMODEL_H
#define TRANSLATIONSMODEL_H

#include <QAbstractListModel>

#include <KConfigGroup>

#include <QSet>

class TranslationsModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QStringList selectedLanguages READ selectedLanguages WRITE setSelectedLanguages NOTIFY selectedLanguagesChanged)
    Q_PROPERTY(QStringList missingLanguages READ missingLanguages NOTIFY missingLanguagesChanged)

    public:
        enum AdditionalRoles {
            LanguageCode = Qt::UserRole + 1,
            IsSelected,
            SelectedPriority,
            IsMissing
        };
        Q_ENUM(AdditionalRoles)

        explicit TranslationsModel(QObject *parent = nullptr);
        ~TranslationsModel() override;

        QHash<int, QByteArray> roleNames() const override;

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;

        QStringList selectedLanguages() const;
        void setSelectedLanguages(const QStringList &languages);

        QStringList missingLanguages() const;

    public Q_SLOTS:
        void moveSelectedLanguage(int from, int to);
        void removeSelectedLanguage(const QString &languageCode);

    Q_SIGNALS:
        void selectedLanguagesChanged() const;
        void missingLanguagesChanged() const;

    private:
        QString languageCodeToName(const QString& languageCode) const;

        KConfigGroup m_config;

        QStringList m_languages;

        QSet<QString> m_installedLanguages;

        QStringList m_selectedLanguages;
        QStringList m_missingLanguages;
};

#endif
