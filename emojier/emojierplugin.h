/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QAbstractItemModel>
#include <QQmlExtensionPlugin>
#include <QSortFilterProxyModel>

#include <qqmlregistration.h>

#include "emojidict.h"
#include "emojiersettings.h"

class AbstractEmojiModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum EmojiRole {
        CategoryRole = Qt::UserRole + 1,
        AnnotationsRole,
    };

    int rowCount(const QModelIndex &parent = {}) const override;

    QVariant data(const QModelIndex &index, int role) const override;

protected:
    QList<Emoji> m_emoji;
};

class EmojiModel : public AbstractEmojiModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QStringList categories MEMBER m_categories CONSTANT)
public:
    enum EmojiRole {
        CategoryRole = Qt::UserRole + 1,
    };

    EmojiModel();

    Q_SCRIPTABLE QString findFirstEmojiForCategory(const QString &category);

private:
    QStringList m_categories;
};

class RecentEmojiModel : public AbstractEmojiModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(int count READ rowCount CONSTANT)
public:
    RecentEmojiModel();

    Q_SCRIPTABLE void includeRecent(const QString &emoji, const QString &emojiDescription);

    Q_INVOKABLE void clearHistory();

private:
    void refresh();

    EmojierSettings m_settings;
};

class CategoryModelFilter : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString category READ category WRITE setCategory)
public:
    QString category() const;

    void setCategory(const QString &category);

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    QString m_category;
};

class SearchModelFilter : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString search READ search WRITE setSearch)
public:
    QString search() const;

    void setSearch(const QString &search);

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    QString m_search;
};

class CopyHelperPrivate : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_NAMED_ELEMENT(CopyHelper)
    QML_SINGLETON
public:
    Q_INVOKABLE static void copyTextToClipboard(const QString &text);
};
