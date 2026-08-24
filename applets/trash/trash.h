/*
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QUrl>

class KCoreDirLister;

class Trash : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool listing READ listing NOTIFY listingChanged)
    Q_PROPERTY(bool emptying READ emptying NOTIFY emptyingChanged)

public:
    explicit Trash(QObject *parent = nullptr);
    ~Trash() override = default;

    int count() const;
    Q_SIGNAL void countChanged();

    bool listing() const;
    Q_SIGNAL void listingChanged(bool listing);

    bool emptying() const;
    Q_SIGNAL void emptyingChanged(bool emptying);

    Q_INVOKABLE void openTrash();
    Q_INVOKABLE void trashUrls(const QList<QUrl> &urls);
    Q_INVOKABLE void emptyTrash();
    Q_INVOKABLE bool canBeTrashed(const QUrl &url) const;
    Q_INVOKABLE QList<QUrl> trashableUrls(const QList<QUrl> &urls) const;

private:
    void setListing(bool listing);

    KCoreDirLister *m_lister;
    bool m_listing = false;
    bool m_emptying = false;
};
