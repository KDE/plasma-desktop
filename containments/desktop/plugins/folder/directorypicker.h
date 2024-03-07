/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QUrl>
#include <qqmlregistration.h>

class QFileDialog;

class DirectoryPicker : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QUrl url READ url NOTIFY urlChanged)

public:
    explicit DirectoryPicker(QObject *parent = nullptr);
    ~DirectoryPicker() override;

    QUrl url() const;

    Q_INVOKABLE void open();

Q_SIGNALS:
    void urlChanged() const;

private Q_SLOTS:
    void dialogAccepted();

private:
    QFileDialog *m_dialog = nullptr;
    QUrl m_url;
};
