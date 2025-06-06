/*
    SPDX-FileCopyrightText: 2025 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "klookandfeel.h"

#include <QAbstractListModel>
#include <QQmlParserStatus>
#include <QUrl>

struct LookAndFeelData {
    QString id;
    QString name;
    QUrl preview;
};

class LookAndFeelModel : public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(KLookAndFeel::Variant variant READ variant WRITE setVariant NOTIFY variantChanged)

public:
    enum Role {
        PackageId = Qt::UserRole + 1,
        Name,
        Preview,
    };

    explicit LookAndFeelModel(QObject *parent = nullptr);

    void classBegin() override;
    void componentComplete() override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent) const override;

    KLookAndFeel::Variant variant() const;
    void setVariant(KLookAndFeel::Variant variant);

    Q_INVOKABLE int indexOf(const QString &packageId) const;

Q_SIGNALS:
    void variantChanged();

private:
    void reload();

    QList<LookAndFeelData> m_lnfs;
    KLookAndFeel::Variant m_variant;
    bool m_complete = false;
};
