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

#include "translationsmodel.h"

#include <KLocalizedString>

#include <QDebug>
#include <QLocale>
#include <QMetaEnum>

TranslationsModel::TranslationsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_installedLanguages = KLocalizedString::availableDomainTranslations("systemsettings");
    m_languages = m_installedLanguages.toList();
}

TranslationsModel::~TranslationsModel()
{
}

QHash<int, QByteArray> TranslationsModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();

    QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("AdditionalRoles"));

    for (int i = 0; i < e.keyCount(); ++i) {
        roles.insert(e.value(i), e.key(i));
    }

    return roles;
}

QVariant TranslationsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_languages.count()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return languageCodeToName(m_languages.at(index.row()));
    } else if (role == LanguageCode) {
        return m_languages.at(index.row());
    } else if (role == IsSelected) {
        return m_selectedLanguages.contains(m_languages.at(index.row()));
    } else if (role == SelectedPriority) {
        return m_selectedLanguages.indexOf(m_languages.at(index.row()));
    } else if (role == IsMissing) {
        return m_missingLanguages.contains(m_languages.at(index.row()));
    }

    return QVariant();
}

int TranslationsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_languages.count();
}

QStringList TranslationsModel::selectedLanguages() const
{
    return m_selectedLanguages;
}


void TranslationsModel::setSelectedLanguages(const QStringList &languages)
{
    if (m_selectedLanguages != languages) {
        QStringList missingLanguages;

        for (const QString& lang : languages) {
            if (!m_installedLanguages.contains(lang)) {
                missingLanguages << lang;
            }
        }

        missingLanguages.sort();

        if (missingLanguages != m_missingLanguages) {
            m_missingLanguages = missingLanguages;
            emit missingLanguagesChanged();
        }

        beginResetModel();

        m_selectedLanguages = languages;
        m_languages = (m_installedLanguages | QSet<QString>::fromList(m_selectedLanguages)).toList();

        endResetModel();

        emit selectedLanguagesChanged();
    }
}

QStringList TranslationsModel::missingLanguages() const
{
    return m_missingLanguages;
}

void TranslationsModel::moveSelectedLanguage(int from, int to)
{
    if (from >= m_selectedLanguages.count() || to >= m_selectedLanguages.count()) {
        return;
    }

    if (from == to) {
        return;
    }

    m_selectedLanguages.move(from, to);

    emit selectedLanguagesChanged();

    auto idx = index(m_languages.indexOf(m_selectedLanguages.at(from)), 0);

    if (idx.isValid()) {
        emit dataChanged(idx, idx, QVector<int>{SelectedPriority});
    }

    idx = index(m_languages.indexOf(m_selectedLanguages.at(to)), 0);

    if (idx.isValid()) {
        emit dataChanged(idx, idx, QVector<int>{SelectedPriority});
    }
}

void TranslationsModel::removeSelectedLanguage(const QString &languageCode)
{
    m_selectedLanguages.removeOne(languageCode);

    emit selectedLanguagesChanged();

    auto idx = index(m_languages.indexOf(languageCode), 0);

    if (idx.isValid()) {
        emit dataChanged(idx, idx, QVector<int>{IsSelected, SelectedPriority});
    }
}

QString TranslationsModel::languageCodeToName(const QString& languageCode) const
{
    const QLocale locale(languageCode);
    const QString &languageName = locale.nativeLanguageName();

    if (languageName.isEmpty()) {
        return languageCode;
    }

    if (languageCode.contains(QStringLiteral("@"))) {
        return i18nc("%1 is language name, %2 is language code name", "%1 (%2)", languageName, languageCode);
    }

    if (locale.name() != languageCode && m_installedLanguages.contains(locale.name())) {
        // KDE languageCode got translated by QLocale to a locale code we also have on
        // the list. Currently this only happens with pt that gets translated to pt_BR.
        if (languageCode == QLatin1String("pt")) {
            return QLocale(QStringLiteral("pt_PT")).nativeLanguageName();
        } else {
            qWarning() << "Language code morphed into another existing language code, please report!" << languageCode << locale.name();
            return i18nc("%1 is language name, %2 is language code name", "%1 (%2)", languageName, languageCode);
        }
    }

    return languageName;
}
