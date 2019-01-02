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

#include <QCollator>
#include <QDebug>
#include <QLocale>
#include <QMetaEnum>

QStringList TranslationsModel::m_languages = QStringList();
QSet<QString> TranslationsModel::m_installedLanguages = QSet<QString>();

TranslationsModel::TranslationsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    if (m_installedLanguages.isEmpty()) {
        m_installedLanguages = KLocalizedString::availableDomainTranslations("systemsettings");
        m_languages = m_installedLanguages.toList();
    }
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
    } else if (role == IsMissing) {
        return false;
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
        }

        qWarning() << "Language code morphed into another existing language code, please report!" << languageCode << locale.name();
        return i18nc("%1 is language name, %2 is language code name", "%1 (%2)", languageName, languageCode);
    }

    return languageName;
}

SelectedTranslationsModel::SelectedTranslationsModel(QObject *parent)
    : TranslationsModel(parent)
{
}

SelectedTranslationsModel::~SelectedTranslationsModel()
{
}

QVariant SelectedTranslationsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_selectedLanguages.count()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return languageCodeToName(m_selectedLanguages.at(index.row()));
    } else if (role == LanguageCode) {
        return m_selectedLanguages.at(index.row());
    } else if (role == IsMissing) {
        return m_missingLanguages.contains(m_selectedLanguages.at(index.row()));
    }

    return QVariant();
}

int SelectedTranslationsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_selectedLanguages.count();
}

QStringList SelectedTranslationsModel::selectedLanguages() const
{
    return m_selectedLanguages;
}

void SelectedTranslationsModel::setSelectedLanguages(const QStringList &languages)
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

        endResetModel();

        emit selectedLanguagesChanged(m_selectedLanguages);
    }
}

QStringList SelectedTranslationsModel::missingLanguages() const
{
    return m_missingLanguages;
}

void SelectedTranslationsModel::move(int from, int to)
{
    if (from >= m_selectedLanguages.count() || to >= m_selectedLanguages.count()) {
        return;
    }

    if (from == to) {
        return;
    }

    const int modelTo = to + (to > from ? 1 : 0);

    const bool ok = beginMoveRows(QModelIndex(), from, from, QModelIndex(), modelTo);

    if (ok) {
        m_selectedLanguages.move(from, to);

        endMoveRows();

        emit selectedLanguagesChanged(m_selectedLanguages);
    }
}

void SelectedTranslationsModel::remove(const QString &languageCode)
{
    if (languageCode.isEmpty()) {
        return;
    }

    int index = m_selectedLanguages.indexOf(languageCode);

    if (index < 0 || m_selectedLanguages.count() < 2) {
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);

    m_selectedLanguages.removeAt(index);

    endRemoveRows();

    emit selectedLanguagesChanged(m_selectedLanguages);
}

AvailableTranslationsModel::AvailableTranslationsModel(QObject *parent)
    : TranslationsModel(parent)
{
}

AvailableTranslationsModel::~AvailableTranslationsModel()
{
}

QVariant AvailableTranslationsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_availableLanguages.count()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return languageCodeToName(m_availableLanguages.at(index.row()));
    } else if (role == LanguageCode) {
        return m_availableLanguages.at(index.row());
    } else if (role == IsMissing) {
        return false;
    }

    return QVariant();
}

int AvailableTranslationsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_availableLanguages.count();
}

QString AvailableTranslationsModel::langCodeAt(int row)
{
    if (row < 0 || row >= m_availableLanguages.count()) {
        return QString();
    }

    return m_availableLanguages.at(row);
}

void AvailableTranslationsModel::setSelectedLanguages(const QStringList &languages)
{
    beginResetModel();

    m_availableLanguages = (m_installedLanguages - QSet<QString>::fromList(languages)).toList();

    QCollator c;
    c.setCaseSensitivity(Qt::CaseInsensitive);

    std::sort(m_availableLanguages.begin(), m_availableLanguages.end(),
        [this, &c](const QString &a, const QString &b) {
            return c.compare(languageCodeToName(a), languageCodeToName(b)) < 0;
        });

    endResetModel();
}
