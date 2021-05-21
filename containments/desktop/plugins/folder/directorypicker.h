/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DIRECTORYPICKER_H
#define DIRECTORYPICKER_H

#include <QObject>
#include <QUrl>

class QFileDialog;

class DirectoryPicker : public QObject
{
    Q_OBJECT

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
    QFileDialog *m_dialog;
    QUrl m_url;
};

#endif
