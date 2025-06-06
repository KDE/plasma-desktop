/*
    SPDX-FileCopyrightText: 2025 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QUrl>

class LookAndFeelMetaData : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString packageId READ package WRITE setPackage NOTIFY packageChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QUrl preview READ preview NOTIFY previewChanged)

public:
    explicit LookAndFeelMetaData(QObject *parent = nullptr);

    QString package() const;
    void setPackage(const QString &package);

    QString name() const;
    QUrl preview() const;

Q_SIGNALS:
    void packageChanged();
    void nameChanged();
    void previewChanged();

private:
    void setName(const QString &name);
    void setPreview(const QUrl &url);

    QString m_package;
    QString m_name;
    QUrl m_preview;
};
