/*
    SPDX-FileCopyrightText: 2018 <furkantokac34@gmail.com>
    SPDX-FileCopyrightText: 2019 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KQuickManagedConfigModule>

#include <QAction>
#include <QJsonArray>
#include <QJsonValue>
#include <QSortFilterProxyModel>

class QStandardItemModel;

class LandingPageData;
class LandingPageGlobalsSettings;

namespace KActivities
{
namespace Stats
{
class ResultModel;
}
}

class MostUsedModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    enum Roles {
        KcmPluginRole = Qt::UserRole + 1000,
    };

    explicit MostUsedModel(QObject *parent = nullptr);
    void setResultModel(KActivities::Stats::ResultModel *model);
    QHash<int, QByteArray> roleNames() const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    // Model when there is nothing from kactivities-stat
    QStandardItemModel *m_defaultModel = nullptr;
    // Model fed by kactivities-stats
    KActivities::Stats::ResultModel *m_resultModel = nullptr;
    mutable QStringList ignoredKCMs;
};

class KCMLandingPage : public KQuickManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(MostUsedModel *mostUsedModel READ mostUsedModel CONSTANT)
    Q_PROPERTY(LandingPageGlobalsSettings *globalsSettings READ globalsSettings CONSTANT)
    Q_PROPERTY(QString defaultLookAndFeelPackage READ defaultLookAndFeelPackage CONSTANT)

public:
    KCMLandingPage(QObject *parent, const KPluginMetaData &metaData);
    ~KCMLandingPage() override
    {
    }

    MostUsedModel *mostUsedModel() const;

    LandingPageGlobalsSettings *globalsSettings() const;
    QString defaultLookAndFeelPackage() const;

    Q_INVOKABLE void openKCM(const QString &kcm);

    Q_INVOKABLE QAction *kcmAction(const QString &storageId);

public Q_SLOTS:
    void save() override;

private:
    LandingPageData *m_data = nullptr;

    MostUsedModel *m_mostUsedModel = nullptr;
};
