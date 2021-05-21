/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef GLOBALACCELMODEL_H
#define GLOBALACCELMODEL_H

#include <QList>

#include "basemodel.h"

class QDBusError;

class KConfigBase;
class KGlobalAccelInterface;
class KGlobalShortcutInfo;

class FilteredShortcutsModel;

class GlobalAccelModel : public BaseModel
{
    Q_OBJECT

public:
    GlobalAccelModel(KGlobalAccelInterface *interface, QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role) const override;

    void addApplication(const QString &desktopFileName, const QString &displayName);

    void exportToConfig(const KConfigBase &config) override;
    void importConfig(const KConfigBase &config) override;

    void load() override;
    void save() override;

Q_SIGNALS:
    void errorOccured(const QString &);

private:
    Component loadComponent(const QList<KGlobalShortcutInfo> &info);
    void removeComponent(const Component &component);
    void genericErrorOccured(const QString &description, const QDBusError &error);

    KGlobalAccelInterface *m_globalAccelInterface;
};

#endif // SHORTCUTSMODEL_H
