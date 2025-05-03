/*
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QUrl>

class Trash : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool emptying READ emptying NOTIFY emptyingChanged)

public:
    explicit Trash(QObject *parent = nullptr);
    ~Trash() override = default;

    bool emptying() const;
    Q_SIGNAL void emptyingChanged(bool emptying);

    Q_INVOKABLE void openTrash();
    Q_INVOKABLE void trashUrls(const QList<QUrl> &urls);
    Q_INVOKABLE void emptyTrash();
    Q_INVOKABLE bool canBeTrashed(const QUrl &url) const;
    Q_INVOKABLE QList<QUrl> trashableUrls(const QList<QUrl> &urls) const;

private:
    bool m_emptying = false;
};
